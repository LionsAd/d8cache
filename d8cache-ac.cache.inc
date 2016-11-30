<?php

require_once DRUPAL_ROOT . '/includes/cache.inc';
require_once __DIR__ . '/d8cache.cache.inc';
require_once __DIR__ . '/includes/core-attachments-collector.inc';

/**
 * Adds reset() and some properties to DrupalAttachmentsCollector().
 */
class D8CacheDrupalAttachmentsCollector extends DrupalAttachmentsCollector {

  /**
   * A count for how many cache IDs this collector should be used.
   *
   * @var int
   */
  public $count = 0;

  /**
   * The stored previous collector.
   *
   * @var \DrupalAttachmentsCollector
   */
  public $previousCollector = NULL;

  /**
   * Reset the attachments in the collector.
   */
  public function reset() {
    $this->attachments = array();
  }

}

/**
 *
 * @ingroup cache
 */
class D8CacheAttachmentsCollector extends D8Cache {

  /**
   * An array of attachment collectors, keyed by cache id.
   *
   * @var array
   */
  protected $attachmentsCollectors;

  /**
   * The currently active attachments collector for this bin.
   *
   * @var array
   */
  protected $currentAttachmentsCollector;

  /**
   * {@inheritdoc}
   */
  public function getMultiple(&$cids, $allow_invalid = FALSE) {
    $cids_copy = $cids;
    // The parent will properly populate $cids, so we can rely on it.
    $cache = parent::getMultiple($cids, $allow_invalid);

    // Unpack the cached data and process attachments.
    foreach ($cache as $cid => $item) {
      if (is_array($item->data) && isset($item->data['#d8cache_data'])) {
        drupal_process_attached($item->data);
        $cache[$cid]->data = $item->data['#d8cache_data'];
      }
    }

    // In case there are no cids left, return.
    if (empty($cids)) {
      return $cache;
    }

    // We have multiple cids, so we need to reset after each cache set.
    $attachments_collector = new D8CacheDrupalAttachmentsCollector();
    $attachments_collector->count = count($cids);
    $attachments_collector->previousCollector = $this->currentAttachmentsCollector;
    $this->currentAttachmentsCollector = $attachments_collector;

    foreach ($cids as $cid) {
      $this->attachmentsCollectors[$cid] = $attachments_collector;
    }

    return $cache;
  }

  /**
   * {@inheritdoc}
   */
  public function set($cid, $data, $expire = Cache::PERMANENT, $tags = array()) {
    $attachments = array();

    if (isset($this->attachmentsCollectors[$cid])) {
      $attachments_collector = $this->attachmentsCollectors[$cid];
      $attachments = $attachments_collector->getAttachments();
      unset($this->attachmentsCollectors[$cid]);

      // Reset the attachments for re-use.
      $attachments_collector->reset();

      $attachments_collector->count--;
      if ($attachments_collector->count == 0) {
        $this->currentAttachmentsCollector = $attachments_collector->previousCollector;
      }
    }

    if (!empty($attachments)) {
      // Create a pseudo render array.
      $data = array(
        '#d8cache_data' => $data,
        '#attached' => $attachments,
      );
    }

    parent::set($cid, $data, $expire, $tags);
  }

  /**
   * Reset the currently active attachemnts collector - if any.
   */
  public function resetCurrentAttachmentsCollector() {
    if ($this->currentAttachmentsCollector) {
      $this->currentAttachmentsCollector->reset();
    }
  }

}
