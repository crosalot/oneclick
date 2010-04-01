<?php

function oneclick_client_theme_select() {
  // Get current list ef themes.
  require_once drupal_get_path('module', 'system').'/system.admin.inc';
  $themes =& _system_theme_list();

  $theme_default = variable_get('theme_default', 'seven');
  $theme_groups  = array();

  foreach ($themes as &$theme) {
    $admin_theme_options[$theme->name] = $theme->info['name'];
    $theme->is_default = ($theme->name == $theme_default);

    // Identify theme screenshot.
    $theme->screenshot = NULL;
    // Create a list which includes the current theme and all its base themes.
    if (isset($themes[$theme->name]->base_themes)) {
      $theme_keys = array_keys($themes[$theme->name]->base_themes);
      $theme_keys[] = $theme->name;
    }
    else {
      $theme_keys = array($theme->name);
    }
    // Look for a screenshot in the current theme or in its closest ancestor.
    foreach (array_reverse($theme_keys) as $theme_key) {
      if (isset($themes[$theme_key]) && file_exists($themes[$theme_key]->info['screenshot'])) {
        $theme->screenshot = array(
          'path' => $themes[$theme_key]->info['screenshot'],
          'alt' => t('Screenshot for !theme theme', array('!theme' => $theme->info['name'])),
          'title' => t('Screenshot for !theme theme', array('!theme' => $theme->info['name'])),
          'attributes' => array('class' => array('screenshot')),
          'getsize' => FALSE,
        );
        break;
      }
    }

    if (empty($theme->status)) {
     // Ensure this theme is compatible with this version of core.
     // Require the 'content' region to make sure the main page
     // content has a common place in all themes.
      $theme->incompatible_core = !isset($theme->info['core']) || ($theme->info['core'] != DRUPAL_CORE_COMPATIBILITY) || (!isset($theme->info['regions']['content']));
      $theme->incompatible_php = version_compare(phpversion(), $theme->info['php']) < 0;
    }
    $query['token'] = drupal_get_token('system-theme-operation-link');
    $theme->operations = array();
    if (!empty($theme->status) || !$theme->incompatible_core && !$theme->incompatible_php) {
      // Create the operations links.
      $query['theme'] = $theme->name;
      if (drupal_theme_access($theme)) {
        $theme->operations[] = array(
          'title' => t('Settings'),
          'href' => 'admin/appearance/settings/' . $theme->name,
          'attributes' => array('title' => t('Settings for !theme theme', array('!theme' => $theme->info['name']))),
        );
      }
      if (!empty($theme->status)) {
        if (!$theme->is_default) {
          $theme->operations[] = array(
            'title' => t('Disable'),
            'href' => 'admin/appearance/disable',
            'query' => $query,
            'attributes' => array('title' => t('Disable !theme theme', array('!theme' => $theme->info['name']))),
          );
          $theme->operations[] = array(
            'title' => t('Set default'),
            'href' => 'admin/appearance/default',
            'query' => $query,
            'attributes' => array('title' => t('Set !theme as default theme', array('!theme' => $theme->info['name']))),
          );
        }
      }
      else {
        $theme->operations[] = array(
          'title' => t('Enable'),
          'href' => 'admin/appearance/enable',
          'query' => $query,
          'attributes' => array('title' => t('Enable !theme theme', array('!theme' => $theme->info['name']))),
        );
        $theme->operations[] = array(
          'title' => t('Enable and set default'),
          'href' => 'admin/appearance/default',
          'query' => $query,
          'attributes' => array('title' => t('Enable !theme as default theme', array('!theme' => $theme->info['name']))),
        );
      }
    }

    // Add notes to default and administration theme.
    $theme->notes = array();
    $theme->classes = array();
    if ($theme->is_default) {
      $theme->classes[] = 'theme-default';
      $theme->notes[] = t('already in use');
    }

    // Sort enabled and disabled themes into their own groups.
    $theme_groups[$theme->status ? 'enabled' : 'disabled'][] = $theme;
  }

  // There are two possible theme groups.
  $theme_group_titles = array(
    'enabled' => format_plural(count($theme_groups['enabled']), 'Enabled theme', 'Enabled themes'),
  );
  if (!empty($theme_groups['disabled'])) {
    $theme_group_titles['disabled'] = format_plural(count($theme_groups['disabled']), 'Disabled theme', 'Disabled themes');
  };

  uasort($theme_groups['enabled'], 'system_sort_themes');
  drupal_alter('oneclick_client_theme_select', $theme_groups);

  return theme('oneclick_client_theme_select', array('theme_groups' => $theme_groups, 'theme_group_titles' => $theme_group_titles));
}

function theme_oneclick_client_theme_select($variables) {
  $theme_groups = $variables['theme_groups'];

  $output = '<div id="system-themes-page">';

  foreach ($variables['theme_group_titles'] as $state => $title) {
    if (!count($theme_groups[$state])) {
      // Skip this group of themes if no theme is there.
      continue;
    }
    // Start new theme group.
    $output .= '<div class="system-themes-list system-themes-list-'. $state .' clearfix"><h2>'. $title .'</h2>';

    foreach($theme_groups[$state] as $theme) {

      // Theme the screenshot.
      $screenshot = $theme->screenshot ? theme('image', $theme->screenshot) : '<div class="no-screenshot">' . t('no screenshot') . '</div>';

      // Localize the theme description.
      $description = t($theme->info['description']);

      // Style theme info
      $notes = count($theme->notes) ? ' (' . join(', ', $theme->notes) . ')' : '';
      $theme->classes[] = 'theme-selector';
      $theme->classes[] = 'clearfix';
      $output .= '<div class="'. join(' ', $theme->classes) .'">' . $screenshot . '<div class="theme-info"><h3>' . $theme->info['name'] . ' ' . (isset($theme->info['version']) ? $theme->info['version'] : '') . $notes . '</h3><div class="theme-description">' . $description . '</div>';

      // Make sure to provide feedback on compatibility.
      if (!empty($theme->incompatible_core)) {
        $output .= '<div class="incompatible">' . t('This version is not compatible with Drupal !core_version and should be replaced.', array('!core_version' => DRUPAL_CORE_COMPATIBILITY)) . '</div>';
      }
      elseif (!empty($theme->incompatible_php)) {
        if (substr_count($theme->info['php'], '.') < 2) {
          $theme->info['php'] .= '.*';
        }
        $output .= '<div class="incompatible">' . t('This theme requires PHP version @php_required and is incompatible with PHP version !php_version.', array('@php_required' => $theme->info['php'], '!php_version' => phpversion())) . '</div>';
      }
      else {
        $output .= theme('links', array('links' => $theme->operations, 'attributes' => array('class' => array('operations', 'clearfix'))));
      }
      $output .= '</div></div>';
    }
    $output .= '</div>';
  }
  $output .= '</div>';
  return $output;
}
