<?php
/**
 * @file
 * Pantheon specific functions for the D8 caching system backport.
 * @todo Remove those once integrated into drops-7.
 */

// -----------------------------------------------------------------------
// Core hooks

if (!function_exists('pantheon_api_emit_cache_tags')) {
  /**
   * Implements hook_emit_cache_tags().
   */
  function pantheon_api_emit_cache_tags($tags) {
    drupal_add_http_header('Surrogate-Key', implode(' ', $tags));
  }
}
 
if (!function_exists('pantheon_api_invalidate_cache_tags')) {
  /**
   * Implements hook_invalidate_cache_tags().
   */
  function pantheon_api_invalidate_cache_tags($tags) {
   if (function_exists( 'pantheon_clear_edge_keys')) {
     pantheon_clear_edge_keys($tags);
   }
 }
}
