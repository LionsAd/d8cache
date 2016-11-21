<?php
/**
 * @file
 * Core functions for the D8 caching system backport.
 */

// -----------------------------------------------------------------------
// Public API

if (!function_exists('drupal_add_cache_tags')) {
function drupal_add_cache_tags(array $tags) {
  // Process the data as attachment if we have an active attachments collector.
  if (function_exists('drupal_has_attachments_collector') && drupal_has_attachments_collector()) {
    drupal_add_attachment('drupal_add_cache_tags', array($tags));
    return;
  }

  $stored_tags = &drupal_static(__FUNCTION__, array());
  $stored_tags = drupal_merge_cache_tags($stored_tags, $tags);
}
}

if (!function_exists('drupal_get_cache_tags')) {
function drupal_get_cache_tags() {
  $stored_tags = &drupal_static('drupal_add_cache_tags', array());

  // Add the rendered cache tag.
  $tags = drupal_merge_cache_tags($stored_tags, array('rendered'));

  sort($tags);
  return $tags;
}
}

if (!function_exists('drupal_get_cache_tags_from_render_array')) {
function drupal_get_cache_tags_from_render_array($build) {
  if (!isset($build['#attached']['drupal_add_cache_tags'])) {
    return array();
  }

  $tags = array();
  foreach ($build['#attached']['drupal_add_cache_tags'] as $key => $parameters) {
    if (!isset($parameters[0])) {
      continue;
    }

    $tags = drupal_merge_cache_tags($tags, $parameters[0]);
  }

  // Add the 'rendered' cache tag.
  $tags = drupal_merge_cache_tags($tags, array('rendered'));

  sort($tags);
  return $tags;
}
}

if (!function_exists('drupal_invalidate_cache_tags')) {
function drupal_invalidate_cache_tags(array $tags) {
  // @todo Add hooks to .api.php.
  drupal_alter('pre_invalidate_cache_tags', $tags);
  module_invoke_all('invalidate_cache_tags', $tags);
}
}

if (!function_exists('drupal_merge_cache_tags')) {
function drupal_merge_cache_tags(array $tags1, array $tags2) {
  return array_unique(array_merge($tags1, $tags2));
}
}

if (!function_exists('drupal_emit_cache_tags')) {
function drupal_emit_cache_tags() {
  $tags = drupal_get_cache_tags();
  // @todo Add hooks to .api.php.
  drupal_alter('pre_emit_cache_tags', $tags);
  module_invoke_all('emit_cache_tags', $tags);
}
}
