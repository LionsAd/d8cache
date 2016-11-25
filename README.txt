Where is the line?
------------------

While in Drupal 8 even a configuration change does invalidate certain cache
tags, that does make less sense in Drupal 7.

Code that is deployed should continue to clear e.g. the 'rendered' cache tag
via:

  drupal_invalidate_tags(array('rendered'));

manually.

The cache tags in Drupal 7 deal only with 'content' not with configuration. And
users and menus are considered content here, too.

The line is drawn where things are edited live by content editors (not admins)
vs. things that should be deployed via features / in code.

Drupal 8 API comparison
-----------------------

* Cache::invalidateTags($tags) => drupal_invalidate_cache_tags($tags)
* onResponseListener() / $response->getCacheTags() => drupal_get_cache_tags()
* CacheableMetadata::fromRenderArray($build) => drupal_get_cache_tags_from_render_array($build)

Frequently asked questions
--------------------------

- Every content change expires all pages. How can I avoid that?

Unfortunately once you add e.g. a recent_content block to the sidebar, d8cache
will add a node_list cache tag, which is cleared whenever an entity of that type
is created, updated or deleted.

To avoid this problem you can implement hook_pre_invalidate_cache_tags_alter()
to remove the node_list cache tag:

/**
 * Implements hook_pre_invalidate_cache_tags_alter().
 */
function mymodule_pre_invalidate_cache_tags_alter(&$tags) {
  $index_tags = array_flip($tags);

  if (isset($index_tags['node_list'])) {
    unset($tags[$index_tags['node_list']]);
  }
}

The best way to expire your pages is to manually clear your cache tags.

- How can I emit a custom header with cache tags for my special Varnish configuration?

You can implement hook_emit_cache_tags() and use drupal_add_http_header, e.g.:

/**
 * Implements hook_emit_cache_tags().
 */
function mymodule_emit_cache_tags($tags) {
  drupal_add_http_header('Surrogate-Key', implode(' ', $tags));
}

- How can I react to cache tag invalidations?

You can implement hook_invalidate_cache_tags() like this:

/**
 * Implements hook_invalidate_cache_tags().
 */
function mymodule_invalidate_cache_tags($tags) {
  mycustom_varnish_clear_cache_tags($tags);
}

- How can I add a custom cache tag?

In the code that shows th

- How can I avoid the block and page cache being invalidated when content changes?

Unfortunately core's internal block and page caches are invalidated when content
changes automatically.

The d8cache module has a built-in configuration option to avoid this and make
cache items PERMANENT instead. In your settings.php use:

$conf['d8cache_cache_options']['cache_block']['cache_max_age'] = CACHE_PERMANENT;

to make all block caches permanent. To set them to 1 hour instead, use:

$conf['d8cache_cache_options']['cache_block']['cache_max_age'] = 3600;

Alternatively you can also use CACHE_PERMANENT (maximum) and then in your code
use:

drupal_add_cache_max_age(3600);

to restrict the max-age further down.

Note that this max-age is bubbled up to cache_page, but not for external
proxies like Varnish.
