<?php
/**
 * @file
 * Entity functions and hooks for the D8 caching system backport.
 */

/**
 * Implements hook_entity_view().
 */
function d8cache_entity_view($entity, $entity_type, $view_mode, $langcode) {
  $tags = _d8cache_entity_get_cache_tags($entity_type, $entity);
  $tags[] = $entity_type . '_view';

  if ($entity_type == 'node' && $view_mode != 'full') {
    // @todo Find a better way to add list cache tags.
    $tags = array_merge($tags, _d8cache_entity_get_list_cache_tags($entity_type));
  }

  $entity->content['#attached']['drupal_add_cache_tags'][][0] = $tags;
}

/**
 * Implements hook_entity_load().
 */
function d8cache_entity_load($entities, $entity_type) {
  $disabled = &drupal_static(__FUNCTION__, FALSE);

  if ($disabled) {
    return;
  }

  $tags = array();
  foreach ($entities as $entity_id => $entity) {
    $tags[] = $entity_type . ':' . $entity_id;
  }

  drupal_add_cache_tags($tags);
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
// Contrib hooks

/**
 * Implements hook_entitycache_load().
 */
function d8cache_entitycache_load($entities, $entity_type) {
  d8cache_entity_load($entities, $entity_type);
}

// -----------------------------------------------------------------------
// Public API

/**
 * Temporarily disable adding of cache tags during entity loading.
 */
function d8cache_disable_cache_tags_for_entity_load($new_status = NULL) {
  $state = &drupal_static('d8cache_entity_load', FALSE);
  $old_state = $state;

  if ($new_state !== NULL) {
    $state = $new_state;
  }

  return $old_state;
}

// -----------------------------------------------------------------------
// Helper functions

function _d8cache_entity_invalidate_cache_tags($entity_type, $entity, $invalidate_list = FALSE) {
  $tags = array();
  if ($invalidate_list) {
    $tags = _d8cache_entity_get_list_cache_tags($entity_type);
  }
  $tags = array_merge($tags, _d8cache_entity_get_cache_tags($entity_type, $entity));

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
