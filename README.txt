Where is the line?
------------------

While in Drupal 8 even a configuration change does invalidate certain cache
tags, that does make less sense in Drupal 7.

Code that is deployed should continue to clear e.g. the 'rendered' cache tag via:

  drupal_invalidate_tags(array('rendered'));

manually.

The cache tags in Drupal 7 deal only with 'content' not with configuration. And 
users and menus are considered content here, too.

The line is drawn where things are edited live by content editors (not admins)
vs. things that should be deployed via features / in code.
