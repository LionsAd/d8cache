<?php
/**
 * @file
 * Core functions for the D8 caching system backport.
 */

// -----------------------------------------------------------------------
// Cache Tags

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

// -----------------------------------------------------------------------
// Cache Max-age

if (!defined('CACHE_MAX_AGE_PERMANENT')) {
  define('CACHE_MAX_AGE_PERMANENT', -1);
}

if (!function_exists('drupal_set_cache_max_age')) {
function drupal_set_cache_max_age($max_age) {
  // Process the data as attachment if we have an active attachments collector.
  if (function_exists('drupal_has_attachments_collector') && drupal_has_attachments_collector()) {
    drupal_add_attachment('drupal_set_cache_max_age', array($max_age));
    return;
  }

  $stored_max_age = &drupal_static(__FUNCTION__, CACHE_MAX_AGE_PERMANENT);
  $stored_max_age = drupal_merge_cache_max_ages($stored_max_age, $max_age);
}
}

if (!function_exists('drupal_get_cache_max_age')) {
function drupal_get_cache_max_age() {
  $stored_max_age = &drupal_static('drupal_set_cache_max_age', CACHE_MAX_AGE_PERMANENT);

  return $stored_max_age;
}
}

if (!function_exists('drupal_merge_cache_max_ages')) {
function drupal_merge_cache_max_ages($a, $b) {
  // If one of the values is CACHE_MAX_AGE_PERMANENT, return the other value.
  if ($a === CACHE_MAX_AGE_PERMANENT) {
    return $b;
  }
  if ($b === CACHE_MAX_AGE_PERMANENT) {
    return $a;
  }

  // If none of the values are CACHE_MAX_AGE_PERMANENT, return the minimum value.
  return min($a, $b);
}
}

// -----------------------------------------------------------------------
// Cache Max-age

if (!function_exists('drupal_get_cacheable_metadata_from_render_array')) {
function drupal_get_cacheable_metadata_from_render_array($build) {
  if (!isset($build['#attached'])) {
    return array();
  }

  $contexts = array();
  $max_age = CACHE_MAX_AGE_PERMANENT;
  $tags = array('rendered');

  // @todo Implement cache contexts.
  if (isset($build['#attached']['drupal_set_cache_max_age'])) {
    foreach ($build['#attached']['drupal_set_cache_max_age'] as $key => $parameters) {
      if (!isset($parameters[0])) {
        continue;
      }

      $max_age = drupal_merge_cache_max_ages($max_age, $parameters[0]);
    }
  }

  if (isset($build['#attached']['drupal_add_cache_tags'])) {
    foreach ($build['#attached']['drupal_add_cache_tags'] as $key => $parameters) {
      if (!isset($parameters[0])) {
        continue;
      }

      $tags = drupal_merge_cache_tags($tags, $parameters[0]);
    }
    sort($tags);
  }

  $cacheable_metadata = array(
    'contexts' => $contexts,
    'max-age' => $max_age,
    'tags' => $tags,
  );

  return $cacheable_metadata;
}
}

if (!function_exists('drupal_emit_cache_max_age')) {
function drupal_emit_cache_max_age() {
  $max_age = drupal_get_cache_max_age();
  // @todo Add hooks to .api.php.
  drupal_alter('pre_emit_cache_max_age', $max_age);
  module_invoke_all('emit_cache_max_age', $max_age);
}
}
