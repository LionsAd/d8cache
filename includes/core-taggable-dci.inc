<?php
/**
 * @file
 * TaggableDrupalCacheInterface functions for the D8 caching system backport.
 */

require_once DRUPAL_ROOT . '/includes/cache.inc';

// -----------------------------------------------------------------------
// Public API

if (!interface_exists('TaggableDrupalCacheInterface', FALSE)) {
/**
 * Defines an interface for a Drupal cache backend, which supports tags.
 */
interface TaggableDrupalCacheInterface extends DrupalCacheInterface {
  /**
   * Stores data in the persistent cache.
   *
   * @param $cid
   *   The cache ID of the data to store.
   * @param $data
   *   The data to store in the cache. Complex data types will be automatically
   *   serialized before insertion. Strings will be stored as plain text and not
   *   serialized. Some storage engines only allow objects up to a maximum of
   *   1MB in size to be stored by default. When caching large arrays or
   *   similar, take care to ensure $data does not exceed this size.
   * @param $expire
   *   (optional) One of the following values:
   *   - CACHE_PERMANENT: Indicates that the item should never be removed unless
   *     explicitly told to using cache_clear_all() with a cache ID.
   *   - CACHE_TEMPORARY: Indicates that the item should be removed at the next
   *     general cache wipe.
   *   - A Unix timestamp: Indicates that the item should be kept at least until
   *     the given time, after which it behaves like CACHE_TEMPORARY.
   * @param $tags
   *   (optional) An array of cache tags, defaults to no tags.
   */
  function set($cid, $data, $expire = CACHE_PERMANENT, $tags = array());
}
}
