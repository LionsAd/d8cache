<?php
/**
 * @file
 * Core functions for the D8 caching system backport.
 */

// -----------------------------------------------------------------------
// Public API

function drupal_add_cache_tags(array $tags) {
  $stored_tags = &drupal_static(__FUNCTION__, array());
  $stored_tags = array_unique(array_merge($stored_tags, $tags));
}

function drupal_get_cache_tags() {
  $stored_tags = &drupal_static('drupal_add_cache_tags', array());
  return $stored_tags;
}

function drupal_invalidate_cache_tags(array $tags) {
  module_invoke_all('invalidate_cache_tags', $tags);
}

function drupal_emit_cache_tags() {
  $tags = drupal_get_cache_tags();
  module_invoke_all('emit_cache_tags', $tags);
}
