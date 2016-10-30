<?php
/**
 * @file
 * Core functions for the D8 caching system backport.
 */

// -----------------------------------------------------------------------
// Public API

function drupal_add_cache_tags(array $tags) {
  $stored_tags = &drupal_static(__FUNCTION__, array());
  $stored_tags = drupal_merge_cache_tags($stored_tags, $tags);
}

function drupal_get_cache_tags() {
  $stored_tags = &drupal_static('drupal_add_cache_tags', array());
  sort($stored_tags);
  return $stored_tags;
}

function drupal_invalidate_cache_tags(array $tags) {
  // @todo Add hooks to .api.php.
  drupal_alter('pre_invalidate_cache_tags', $tags);
  module_invoke_all('invalidate_cache_tags', $tags);
}

function drupal_merge_cache_tags(array $tags1, array $tags2) {
  return array_unique(array_merge($tags1, $tags2));
}

function drupal_emit_cache_tags() {
  $tags = drupal_get_cache_tags();
  // @todo Add hooks to .api.php.
  drupal_alter('pre_emit_cache_tags', $tags);
  module_invoke_all('emit_cache_tags', $tags);
}
