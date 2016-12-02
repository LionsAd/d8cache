<?php

require_once DRUPAL_ROOT . '/includes/cache.inc';
require_once __DIR__ . '/includes/core.inc';
require_once __DIR__ . '/includes/core-taggable-dci.inc';

/**
 * Defines a Drupal 8 cacheable metadata aware cache backend.
 *
 * @ingroup cache
 */
class D8Cache implements TaggableDrupalCacheInterface {

  /**
   * @var string
   */
  protected $bin;

  /**
   * The cache backend.
   *
   * @var \DrupalCacheInterface
   */
  protected $backend;

  /**
   * The cache bin specific configuration.
   *
   * @var array
   */
  protected $configuration;

  /**
   * Constructs a Drupal8CacheBackend object.
   *
   * @param string $bin
   *   The cache bin for which the object is created.
   */
  public function __construct($bin) {
    global $conf;

    $this->bin = $bin;

    $class = variable_get('d8cache_cache_class_' . $bin);
    if (!isset($class)) {
      $class = variable_get('d8cache_cache_default_class');
    }
    if (!isset($class)) {
      $class = variable_get('cache_default_class', 'DrupalDatabaseCache');
    }

    $this->backend = new $class($bin);
    $this->configuration = array();

    $configuration = variable_get('d8cache_cache_options', array());
    if (isset($configuration[$bin])) {
      $this->configuration = $configuration[$bin];
    }
  }

  /**
   * {@inheritdoc}
   */
  public function get($cid, $allow_invalid = FALSE) {
    $cids = array($cid);
    $cache = $this->getMultiple($cids, $allow_invalid);
    return reset($cache);
  }

  /**
   * {@inheritdoc}
   */
  public function getMultiple(&$cids, $allow_invalid = FALSE) {
    if ($this->bin == 'cache_page' && !function_exists('d8cache_cache_tags_is_valid')) {
      // In case that we use the internal page cache we need to load the
      // module early.
      drupal_load('module', 'd8cache');
    }

    $cache = array();
    $cids_map = array_flip($cids);

    foreach ($this->backend->getMultiple($cids) as $cid => $item) {
      // This should never happen.
      if (!isset($cids_map[$cid])) {
        continue;
      }

      $data = $item->data;
      $expire = isset($data->d8cache_expire) ? $data->d8cache_expire : $item->expire;

      // Check expire time.
      $item->valid = $expire === CACHE_PERMANENT || $expire === CACHE_TEMPORARY || $expire >= REQUEST_TIME;

      // Is this packed data?
      if ($data instanceof stdClass && isset($data->d8cache_tags)) {
        // Check if the cache tags are valid.
        if (!d8cache_cache_tags_is_valid($data->d8cache_checksum, $data->d8cache_tags)) {
          $item->valid = FALSE;
        }

        $item->data = $data->d8cache_data;
      }

      if (!$allow_invalid && !$item->valid) {
        continue;
      }

      $cache[$cid] = $item;
      unset($cids_map[$cid]);
    }

    // Re-calculate the cids property.
    $cids = array_keys($cids_map);

    return $cache;
  }

  /**
   * {@inheritdoc}
   */
  public function set($cid, $data, $expire = CACHE_PERMANENT, $tags = array()) {
    // Allow to override the TTL for a whole bin.
    if (isset($this->configuration['ttl'])) {
      if ($this->configuration['ttl'] == CACHE_PERMANENT || $this->configuration['ttl'] == CACHE_TEMPORARY) {
        $expire = $this->configuration['ttl'];
      }
      else {
        $expire = REQUEST_TIME + $this->configuration['ttl'];
      }
    }

    // Special case cache_page.
    if ($this->bin == 'cache_page') {
      $page_cache_tags = &drupal_static('d8cache_emit_cache_tags', array());
      $page_cache_max_age = &drupal_static('d8cache_emit_cache_max_age', CACHE_MAX_AGE_PERMANENT);

      if (!empty($page_cache_tags)) {
        $tags = drupal_merge_cache_tags($tags, $page_cache_tags);
      }

      if ($page_cache_max_age != CACHE_MAX_AGE_PERMANENT) {
        $expire = $this->mergeExpireWithMaxAge($expire, $page_cache_max_age);
      }
    }

    // Render arrays.
    if (is_array($data)
        && isset($data['#attached'])
        && (isset($data['#attached']['drupal_add_cache_tags']) || isset($data['#attached']['drupal_set_cache_max_age']))) {
      $cacheable_metadata = drupal_get_cacheable_metadata_from_render_array($data);

      if (!empty($cacheable_metadata['tags'])) {
        $tags = drupal_merge_cache_tags($tags, $cacheable_metadata['tags']);
      }

      if ($cacheable_metadata['max-age'] != CACHE_MAX_AGE_PERMANENT) {
        $expire = $this->mergeExpireWithMaxAge($expire, $cacheable_metadata['max-age']);
      }
    }

    // No tags, present, just continue normally.
    if (empty($tags)) {
      $this->backend->set($cid, $data, $expire);
      return;
    }

    // Does the backend support tags natively?
    if ($this->backend instanceof TaggableDrupalCacheInterface) {
      $this->backend->set($cid, $data, $expire, $tags);
    }
    else {
      $checksum = d8cache_cache_tags_get_current_checksum($tags);

      $data = (object) array(
        'd8cache_tags' => $tags,
        'd8cache_checksum' => $checksum,
        'd8cache_expire' => $expire,
        'd8cache_data' => $data,
      );

      $this->backend->set($cid, $data, $expire);
    }
  }

  /**
   * {@inheritdoc}
   */
  public function clear($cid = NULL, $wildcard = FALSE) {
    $this->backend->clear($cid, $wildcard);
  }

  /**
   * {@inheritdoc}
   */
  function isEmpty() {
    // It is impossible to determine this as we cannot list all items in a
    // consistent way as it depends on the backend used.
    return FALSE;
  }

  /**
   * Merges an max-age value with an expire timestamp.
   *
   * @param int $expire
   *   A unix timestamp when this item will expire or one of the CACHE_*
   *   constants.
   * @param int $max_age
   *   A max-age ttl value like an integer or CACHE_MAX_AGE_PERMANENT
   *
   * @return int
   *   A unix timestamp when this item will expire or one of the CACHE_*
   *   constants.
   */
  protected function mergeExpireWithMaxAge($expire, $max_age) {
    // Do not mess with temporary items.
    if ($expire == CACHE_TEMPORARY) {
      return $expire;
    }

    // In case $max_age is PERMANENT return $expire as is.
    if ($max_age === CACHE_MAX_AGE_PERMANENT) {
      return $expire;
    }

    // If $expire is permanent return the numeric ttl.
    if ($expire == CACHE_PERMANENT) {
      return REQUEST_TIME + $max_age;
    }

    // In all other cases return the minimum of ttl($expire) and $max_age.
    return REQUEST_TIME + min(REQUEST_TIME - $expire, $max_age);
  }
}
