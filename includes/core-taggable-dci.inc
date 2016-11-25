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

  /**
   * Returns data from the persistent cache.
   *
   * Data may be stored as either plain text or as serialized data. cache_get()
   * will automatically return unserialized objects and arrays.
   *
   * @param $cid
   *   The cache ID of the data to retrieve.
   * @param $allow_invalid
   *   (optional) If TRUE, a cache item may be returned even if it is expired or
   *   has been invalidated. Such items may sometimes be preferred, if the
   *   alternative is recalculating the value stored in the cache, especially
   *   if another concurrent request is already recalculating the same value.
   *   The "valid" property of the returned object indicates whether the item is
   *   valid or not. Defaults to FALSE.
   *
   * @return
   *   The cache or FALSE on failure.
   */
  function get($cid, $allow_invalid = FALSE);

  /**
   * Returns data from the persistent cache when given an array of cache IDs.
   *
   * @param $cids
   *   An array of cache IDs for the data to retrieve. This is passed by
   *   reference, and will have the IDs successfully returned from cache
   *   removed.
   * @param $allow_invalid
   *   (optional) If TRUE, cache items may be returned even if they have expired
   *   or been invalidated. Such items may sometimes be preferred, if the
   *   alternative is recalculating the value stored in the cache, especially
   *   if another concurrent thread is already recalculating the same value. The
   *   "valid" property of the returned objects indicates whether the items are
   *   valid or not. Defaults to FALSE.
   *
   * @return
   *   An array of the items successfully returned from cache indexed by cid.
   */
  function getMultiple(&$cids, $allow_invalid = FALSE);
}
}
