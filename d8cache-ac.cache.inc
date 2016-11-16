<?php

require_once DRUPAL_ROOT . '/includes/cache.inc';
require_once __DIR__ . '/d8cache.cache.inc';

/**
 *
 * @ingroup cache
 */
class D8CacheAttachmentsCollector extends D8Cache {

  /**
   * {@inheritdoc}
   */
  public function getMultiple(&$cids) {
    $cids_copy = $cids;
    // The parent will properly populate $cids, so we can rely on it.
    $cache = parent::getMultiple($cids);

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

    // In case there is just one cid left, we can use the easy way.
    if (count($cids) == 1) {
      $cid = reset($cids);
      $this->attachmentsCollectors[$cid] = new DrupalAttachmentsCollector();
      return $cache;
    }

    // We have multiple cids, so we need to reset after each cache set.
    $attachments_collector = new DrupalAttachmentsCollector();

    $counter = (object) array(
      'attachmentsCollector' => $attachments_collector,
      'count' => count($cids),
    );

    foreach ($cids as $cid) {
      $this->attachmentsCollectorGroup[$cid] = $counter;
    }

    return $cache;
  }

  /**
   * {@inheritdoc}
   */
  public function set($cid, $data, $expire = Cache::PERMANENT, $tags = array()) {
    $attachments_collector = NULL;
    if (isset($this->attachmentsCollectorGroup[$cid])) {
      $counter = $this->attachmentsCollectorGroup[$cid];
      unset($this->attachmentsCollectorGroup[$cid]);

      $attachments_collector = $counter->attachmentsCollector;
      $counter->attachmentsCollector = NULL;

      if ($counter->count > 0) {
        $counter->count--;
        $counter->attachmentsCollector = new DrupalAttachmentsCollector();
      }
    }
    elseif (isset($this->attachmentsCollectors[$cid])) {
      $attachments_collector = $this->attachmentsCollectors[$cid];
      $this->attachmentsCollectors[$cid] = NULL;
    }

    if (isset($attachments_collector)) {
      $attachments = $attachments_collector->getAttachments();
      $attachments_collector = NULL;

      // Create a pseudo render array.
      $data = array(
        '#d8cache_data' => $data,
        '#attached' => $attachments,
      );
    }

    parent::set($cid, $data, $expire, $tags);
  }

}
