<?php
/**
 * @file
 * Entity functions and hooks for the D8 caching system backport.
 */

/**
 * Implements hook_entity_view().
 */
function d8cache_entity_view($entity, $entity_type, $view_mode, $langcode) {
  $entity->content['#attached']['drupal_add_cache_tags'][][] = _d8cache_entity_get_cache_tags($entity_type, $entity);
  if ($view_mode != 'full') {
    $entity->content['#attached']['drupal_add_cache_tags'][][] = _d8cache_entity_get_list_cache_tags($entity_type);
  }
}

/**
 * Implements hook_entity_insert().
 */
function d8cache_entity_insert($entity, $entity_type) {
  _d8cache_entity_invalidate_cache_tags($entity_type, $entity, TRUE);
}

/**
 * Implements hook_entity_update().
 */
function d8cache_entity_update($entity, $entity_type) {
  _d8cache_entity_invalidate_cache_tags($entity_type, $entity, FALSE);
}

/**
 * Implements hook_entity_delete().
 */
function d8cache_entity_delete($entity, $entity_type) {
  _d8cache_entity_invalidate_cache_tags($entity_type, $entity, TRUE);
}

// -----------------------------------------------------------------------
// Helper functions

function _d8cache_entity_invalidate_cache_tags($entity_type, $entity, $invalidate_list = FALSE) {
  $tags = array();
  if ($invalidate_list) {
    $tags[] = _d8cache_entity_get_list_cache_tags($entity_type);
  }
  $tags[] = _d8cache_entity_get_cache_tags($entity_type, $entity);

  drupal_invalidate_cache_tags($tags);
}

function _d8cache_entity_get_list_cache_tags($entity_type) {
  return array(
    $entity_type . '_list',
  );
}

function _d8cache_entity_get_cache_tags($entity_type, $entity) {
  list($entity_id,,) = entity_extract_ids($entity_type, $entity);
  return array(
    $entity_type . ':' . $entity_id,
  );
}
