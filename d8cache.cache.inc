<?php

require_once DRUPAL_ROOT . '/includes/cache.inc';
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
  }

  /**
   * {@inheritdoc}
   */
  public function get($cid) {
    $cids = array($cid);
    $cache = $this->getMultiple($cids);
    return reset($cache);
  }

  /**
   * {@inheritdoc}
   */
  public function getMultiple(&$cids) {
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

      // Is this packed data?
      if ($data instanceof stdClass && isset($data->d8cache_tags)) {
        // @todo: Check validity of item here.
        $valid = TRUE;

        // Check if the cache tags are valid.
        if (!d8cache_cache_tags_is_valid($data->d8cache_checksum, $data->d8cache_tags)) {
          $valid = FALSE;
        }

        // Check if this item was expired already.
        $expire = $data->d8cache_expire;
        if ($expire !== CACHE_PERMANENT && $expire !== CACHE_TEMPORARY && $expire <= REQUEST_TIME) {
          $valid = FALSE;
        }

        if (!$valid) {
          continue;
        }

        $item->data = $data->d8cache_data;
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

    // Add CACHE_PERMANENT for demo purposes.
    $expire = CACHE_PERMANENT;

    // Special case cache_page.
    if ($this->bin == 'cache_page') {
      $page_cache_tags = &drupal_static('d8cache_emit_cache_tags', array());

      $tags = drupal_merge_cache_tags($tags, $page_cache_tags);
    }

    // Render arrays.
    if (is_array($data) && isset($data['#attached']['drupal_add_cache_tags'])) {
      $tags = drupal_merge_cache_tags($tags, drupal_get_cache_tags_from_render_array($data));
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

}
