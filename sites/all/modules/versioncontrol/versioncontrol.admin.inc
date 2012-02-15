<?php
/**
 * @file
 * Version Control API - An interface to version control systems
 * whose functionality is provided by pluggable back-end modules.
 *
 * This file contains the administrative user interface
 * and repositories.
 *
 * Copyright 2006, 2007 Derek Wright ("dww" , http://drupal.org/user/46549)
 * Copyright 2007, 2008, 2009 by Jakob Petsovits ("jpetso", http://drupal.org/user/56020)
 */

/**
 * Form callback for "admin/settings/versioncontrol-settings/general":
 * Provide a form for settings of Version Control API itself.
 */
function versioncontrol_admin_settings(&$form_state) {
  $form = array();
  $presets = _versioncontrol_get_string_presets();

  $form['#id'] = 'versioncontrol-settings-form';

  $form['versioncontrol_email_address'] = array(
    '#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#default_value' => variable_get('versioncontrol_email_address', 'versioncontrol@example.com'),
    '#description' => t('The e-mail address of the VCS administrator.'),
    '#weight' => -10,
  );

  $form['versioncontrol_register_form'] = array(
    '#type' => 'fieldset',
    '#title' => t('Account registration form strings'),
    '#description' => t('The following messages are shown on the !repository-selection-page that leads to the account registration form.', array('!repository-selection-page' => l(t('repository selection page'), 'versioncontrol/register/demo'))),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 0,
  );
  $form['versioncontrol_register_form']['versioncontrol_registration_message_unauthorized'] = array(
    '#type' => 'textarea',
    '#title' => t('Message to unauthorized users'),
    '#description' => t('Message to show to anonymous users and to users without the \'use version control systems\' permission.'),
    '#default_value' => variable_get(
      'versioncontrol_registration_message_unauthorized',
      $presets['versioncontrol_registration_message_unauthorized']
    ),
  );
  $form['versioncontrol_register_form']['versioncontrol_registration_message_authorized'] = array(
    '#type' => 'textarea',
    '#title' => t('Message to registering users'),
    '#description' => t('Message to show to users who are in the process of selecting a repository in order to create a VCS account. If there\'s only one repository on this site, users will never get to see this message.'),
    '#default_value' => variable_get(
      'versioncontrol_registration_message_authorized',
      $presets['versioncontrol_registration_message_authorized']
    ),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 100,
  );

  return $form;
}

/**
 * Submit handler for the authorization settings form.
 * system_settings_form() is left out because it's nasty for modules that
 * hook into this form and don't want to use automatic variable saving.
 */
function versioncontrol_admin_settings_submit($form, &$form_state) {
  $value_names = array(
    'versioncontrol_email_address',
    'versioncontrol_registration_message_unauthorized',
    'versioncontrol_registration_message_authorized',
  );
  foreach ($value_names as $name) {
    variable_set($name, $form_state['values'][$name]);
  }
}

function versioncontrol_admin_views_sets_edit(&$form_state) {
  ctools_include('dependent');
  $form = array();

  $form['info'] = array(
    '#type' => 'markup',
    '#value' => t("This form allows you to manage Version Control API's Views sets. When a user requests a page with Views-driven VCAPI data, VCAPI uses this system to pick a backend-specific view (if appropriate). Backend-specific views always contain richer data than their generic cross-backend counterparts."),
  );

  // get the assembled set plugin objects & ensure other statics are populated
  $sets = versioncontrol_get_views_sets();
  // get the statics, but only copies, no need to write to them now
  $set_infos = ctools_get_plugins('versioncontrol', 'views_sets');
  $db_set_data = ctools_static('versioncontrol_views_sets_db_data', array());
  $all_views = drupal_map_assoc(array_keys(views_get_all_views()));
  $base_options = array('' => t('Select a view')) + $all_views;
  $backends = versioncontrol_get_backends();

  foreach ($set_infos as $set_name => $info) {
    $set = $sets[$set_name];
    $form['set_infos'] = array(
      '#type' => 'value',
      '#value' => $set_infos,
    );
    // Create a fieldset for each set.
    $form[$set_name] = array(
      '#type' => 'fieldset',
      '#title' => $info['title'],
      '#description' => $info['description'],
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
    );

    $form[$set_name]['base'] = array(
      '#type' => 'textfield',
      '#disabled' => TRUE,
      '#title' => t('Base view'),
      '#description' => t('The base view is a fallback. It is only used if no other view is appropriate.'),
      '#default_value' => $set->getBaseView(),
    );

    foreach ($backends as $vcs => $backend) {
      $current_view = $from_db = $from_backend = $from_owner = $from_base = FALSE;
      if (!empty($db_set_data[$set_name][$vcs])) {
        $current_view = $default_selected_view = $db_set_data[$set_name][$vcs];
        $from_db = TRUE;
      }
      else {
        $default_selected_view = '';
      }

      // Init the from description to the base view.
      $from_description = t("No other default was specified, so the base view, <em>'!view_name'</em>, will act as the default.", array('!view_name' => $set->getBaseView()));

      if (!empty($backend->defaultViews[$set_name])) {
        $current_view = $from_db ? $current_view : $backend->defaultViews[$set_name];
        $from_description = t("The view <em>'!view_name'</em> has been specified by the backend as the default view for this set.", array('!view_name' => $current_view));
      }

      $set_default_views = $set->getDefaultViews();
      if (!empty($set_default_views[$vcs])) {
        $current_view = $from_db ? $current_view : $set_default_views[$vcs];
        $from_description = t("The view <em>'!view_name'</em> has been specified by the module implementing the set as the default view for this set.", array('!view_name' => $current_view));
      }

      if (empty($current_view)) {
        // if $current_view still isn't set, the base view is king
        $current_view = $set->getBaseView();
      }

      if (empty($base_options[$current_view])) {
        drupal_set_message("Unknown view '$current_view' currently assigned for backend '$vcs' on set '$set_name'. Attempting to use this set will cause nasty errors.", 'error');
      }

      $form[$set_name][$vcs] = array(
        '#type' => 'fieldset',
        '#title' => $backend->name,
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#tree' => TRUE,
      );

      $form[$set_name][$vcs]['default'] = array(
        '#type' => 'checkbox',
        '#title' => t('Use the default view for this backend.'),
        '#description' => $from_description,
        '#default_value' => !$from_db,
        '#id' => "$set_name-$vcs-default",
      );

      $form[$set_name][$vcs]['override'] = array(
        '#type' => 'select',
        '#title' => t('Select a view'),
        '#options' => $base_options,
        '#default_value' => $default_selected_view,
        '#process' => array('ctools_dependent_process'),
        '#dependency' => array("$set_name-$vcs-default" => array(0)),
      );
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save views sets'),
  );

  return $form;
}

function versioncontrol_admin_views_sets_edit_validate($form, &$form_state) {
  $backends = versioncontrol_get_backends();
  foreach ($form_state['values']['set_infos'] as $set_name => $info) {
    foreach ($backends as $vcs => $backend) {
      $values = $form_state['values'][$set_name][$vcs];
      if (empty($values['default']) && empty($values['override'])) {
        // If bypassing the default, a view must be selected.
        form_error($form[$set_name][$vcs]['override'], 'If not using the default view, a view must be selected.');
      }
    }
  }
}

function versioncontrol_admin_views_sets_edit_submit($form, &$form_state) {
  // Clear the table.
  db_delete('versioncontrol_views_sets')->execute();
  $backends = versioncontrol_get_backends();

  $query = db_insert('versioncontrol_views_sets')->fields(array('views_set', 'vcs', 'view_name'));
  $do_insert = FALSE;
  // $backends = versioncontrol_get_backends();
  foreach ($form_state['values']['set_infos'] as $set_name => $info) {
    foreach ($backends as $vcs => $backend) {
      $values = $form_state['values'][$set_name][$vcs];
      if (empty($values['default'])) {
        $do_insert = TRUE;
        $query->values(array($set_name, $vcs, $values['override']));
      }
    }
  }

  if ($do_insert) {
    // Only do the insert if we actually have some values to save.
    $query->execute();
  }

  ctools_static_reset('versioncontrol_views_sets_assembled');
  ctools_static_reset('versioncontrol_views_sets_db_data');
}

/**
 * Form callback for "admin/settings/versioncontrol-settings/plugins":
 * Provide a form for settings about Version Control API plugins.
 */
function versioncontrol_admin_settings_plugins(&$form_state) {
  ctools_include('plugins');
  $form = array();
  $plugin_entity_types = versioncontrol_plugins_get_information();
  // view_sets have its own settings page, so avoid it
  unset($plugin_entity_types['view']);

  foreach ($plugin_entity_types as $plugin_entity_type => $plugin_types) {
    foreach ($plugin_types as $plugin_type => $plugin_type_data) {
      $form['versioncontrol_plugin_' . $plugin_type] = array(
        '#type' => 'fieldset',
        '#title' => $plugin_type_data['name'],
      );

      $variable = $plugin_type_data['default_variable'];
      $callback = $plugin_type_data['fetcher'];
      $form['versioncontrol_plugin_' . $plugin_type][$variable] = array(
        '#type' => 'select',
        '#title' => t('Default plugin'),
        '#default_value' => variable_get($variable, 0),
        '#options' => function_exists($callback) ? array( 0 => t('Do not set default')) + $callback() : array(),
      );
    }
  }

  // Special configuration for webviewer_url_handler plugin
  // TODO make this more dynamic, so we can avoid a special case
  $backends = versioncontrol_get_backends();
  foreach ($backends as $vcs => $backend) {
    foreach (ctools_get_plugins('versioncontrol', 'webviewer_url_handlers') as $machine_name => $plugin) {
      $variable = 'versioncontrol_repository_' . $vcs . '_base_url_' . $machine_name;
      $form['versioncontrol_plugin_webviewer_url_handlers'][$variable] = array(
        '#type' => 'textfield',
        '#title' => t('%vcs_name: Default base URL for %plugin_name', array('%vcs_name' => $backend->name, '%plugin_name' => $plugin['title'])),
        '#default_value' => variable_get($variable, ''),
      );
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 100,
  );

  return $form;
}

function versioncontrol_admin_settings_plugins_submit($form, &$form_state) {
  ctools_include('plugins');
  $plugin_entity_types = versioncontrol_plugins_get_information();
  // view_sets have its own settings page, so avoid it
  unset($plugin_entity_types['view']);

  foreach ($plugin_entity_types as $plugin_entity_type => $plugin_types) {
    foreach ($plugin_types as $plugin_type => $plugin_type_data) {
      $key = $plugin_type_data['default_variable'];
      if ($form_state['values'][$key] == '0') {
        variable_del($key);
      }
      else {
        variable_set($key, $form_state['values'][$key]);
      }
    }
  }

  // Special configuration for webviewer_url_handler plugin
  // TODO make this more dynamic, so we can avoid a special case
  $backends = versioncontrol_get_backends();
  foreach ($backends as $vcs => $backend) {
    foreach (ctools_get_plugins('versioncontrol', 'webviewer_url_handlers') as $machine_name => $plugin) {
      $key = 'versioncontrol_repository_' . $vcs . '_base_url_' . $machine_name;
      if (empty($form_state['values'][$key])) {
        variable_del($key);
      }
      else {
        variable_set($key, $form_state['values'][$key]);
      }
    }
  }
}

/**
 * Form callback for "admin/content/versioncontrol-repositories":
 * A simple list of repositories, sorted by version control system.
 */
function versioncontrol_admin_repository_list(&$form_state) {
  $form = array();
  $backends = versioncontrol_get_backends();
  $set = versioncontrol_get_views_set('repositories_admin');
  foreach ($backends as $vcs => $backend) {
    $title = t('@vcs repositories', array('@vcs' => $backend->name));
    $form[$vcs] = array(
      '#value' => '<h4>'. $title .'</h4>',
    );
    $form[$vcs . '_repositories'] = array(
      '#value' => views_embed_view($set->getViewNameByBackend($backend), 'default', $vcs),
    );
  }
  return $form;
}

/**
 * Form callback for
 * 'admin/content/versioncontrol-repositories/edit/%versioncontrol_repository'
 * and "admin/content/versioncontrol-repositories/add-$vcs":
 * Provide a form to edit or add a repository.
 *
 * @param $repository
 *   The repository that is to be edited, or FALSE
 *   if the repository doesn't exist yet and should be added.
 * @param $vcs
 *   If $repo_id is NULL, this should be the unique identification string
 *   of the backend for which the repository should be added.
 *   Otherwise, this value is ignored.
 */
function versioncontrol_admin_repository_edit(&$form_state, $repository, $vcs = NULL) {
  $backends = versioncontrol_get_backends();
  $repository_exists = ($repository !== FALSE);

  if (!$repository_exists && !isset($backends[$vcs])) {
    drupal_goto('admin/content/versioncontrol-repositories');
    // drupal_goto() calls exit() so script execution ends right here
  }

  $form = array();
  $form['#id'] = 'versioncontrol-repository-form';

  if ($repository_exists) {
    $form['#repository'] = $repository;
  }
  $form['#vcs'] = $repository_exists ? $repository->vcs : $vcs;
  $form['#original_name'] = $repository_exists ? $repository->name : 0;

  $form['repository_information'] = array(
    '#type' => 'fieldset',
    '#title' => t('Repository information'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => 0,
  );
  $form['repository_information']['repo_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Repository name'),
    '#description' => t('A label for the repository that will be used in all user visible messages.'),
    '#default_value' => $repository_exists ? $repository->name : '',
    '#required' => TRUE,
    '#weight' => 0,
    '#size' => 40,
    '#maxlength' => 255,
  );
  $form['repository_information']['root'] = array(
    '#type' => 'textfield',
    '#title' => t('Repository root'),
    '#description' => t('The location of the repository\'s root directory.'),
    '#default_value' => $repository_exists ? $repository->root : '',
    '#weight' => 5,
    '#size' => 60,
    '#maxlength' => 255,
  );

  $form['repository_information']['update_method'] = array(
    '#type' => 'radios',
    '#title' => t('Update method'),
    '#description' => t('The way Version Control API would know about new changes on the tracked VCS.'),
    '#default_value' => $repository_exists
                        ? $repository->update_method
                        : VERSIONCONTROL_UPDATE_LOG_PARSE_ON_CRON,
    '#weight' => 9,
    '#options' => array(
      VERSIONCONTROL_UPDATE_LOG_PARSE_ON_CRON => t('Automatic log retrieval at cron.'),
      VERSIONCONTROL_UPDATE_INDEPENDENT_EXTERNAL_SCRIPTS => t('Use external script to insert data.'),
    ),
  );

  $text = t('Versioncontrol can map commit activity in this repository to user accounts. These options allow you to select the logic that should be used to perform the mapping between Drupal users and raw VCS data.');
  $text .= '<br/><br/>';
  $text .= t('Versioncontrol internally allows for both "authors" and "committers" to be tracked on each commit, so you can choose distinct mapping logic for each.');

  $form['user_mapping'] = array(
    '#type' => 'fieldset',
    '#title' => t('User mapping'),
    '#description' => $text,
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => 5,
  );

  $form['user_mapping']['author_mapper'] = array(
    '#type' => 'radios',
    '#title' => t('Author mapping'),
    '#description' => t('The mapping logic to be used for "author" data. All VCSes have some sort of author field.'),
    '#default_value' => $repository_exists ? $repository->plugins['author_mapper'] : variable_get('versioncontrol_repository_plugin_default_user_mapping_methods', 'none'),
    '#options' => versioncontrol_user_mapping_methods_get_names(),
  );

  $form['user_mapping']['committer_mapper'] = array(
    '#type' => 'radios',
    '#title' => t('Committer mapping'),
    '#description' => t('The mapping logic to be used for "committer" data. Only some VCSes distinguish between author and committer. For most use cases, it is recommended best to set this to use the same plugin as for author mapping, especially if using "None (No mapping)" for author mapping.'),
    '#default_value' => $repository_exists ? $repository->plugins['committer_mapper'] : variable_get('versioncontrol_repository_plugin_default_user_mapping_methods', 'none'),
    '#options' => versioncontrol_user_mapping_methods_get_names(),
  );

  $form['user_authentication'] = array(
    '#type' => 'fieldset',
    '#title' => t('User authentication'),
    '#description' => t('Versioncontrol can authenticate/validate VCS information based on its entities and the user trying to read or write to this repository. If the authentication interface selected here is properly checked from server-side repository hooks, it can properly control access to the repository.'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => 5,
  );

  $form['user_authentication']['auth_handler'] = array(
    '#type' => 'radios',
    '#title' => t('Authentication & access'),
    '#description' => t('The authentication logic that should govern access to this repository. Note that Version Control API by itself provides very little enforcement of these ACLs. The expectation is that higher-level modules will use the interface.'),
    '#default_value' => $repository_exists ? $repository->plugins['auth_handler'] : variable_get('versioncontrol_repository_plugin_default_auth_handler', 'ffa'),
    '#options' => versioncontrol_auth_handlers_get_names(),
  );

  $repo_url_handler = $repository_exists ? $repository->getUrlHandler() : NULL;

  $form['repository_urls'] = array(
    '#type' => 'fieldset',
    '#title' => t('Repository browser URL handler'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => 5,
  );
  $form['repository_urls']['base_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Base URL'),
    '#default_value' => isset($repo_url_handler) ? $repo_url_handler->baseUrl : '',
    '#size' => 40,
    '#maxlength' => 255,
    '#description' => t('The URL that let you see the repository web viewer. Do not use a trailing slash.'),
  );
  $form['repository_urls']['url_handler'] = array(
    '#type' => 'radios',
    '#title' => t('Web viewer URL handler'),
    '#description' =>  t('The handler which will  provide the URLs that will be used to add links to item and commit displays such as the commit log.'),
    '#default_value' => $repository_exists ? $repository->plugins['webviewer_url_handler'] : variable_get('versioncontrol_repository_plugin_default_webviewer_url_handler', 'none'),
    '#options' => versioncontrol_webviewer_url_handlers_get_names($form['#vcs']),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save repository'),
    '#weight' => 100,
  );

  return $form;
}

/**
 * Add or update the repository when the add/edit form is submitted.
 */
function versioncontrol_admin_repository_edit_submit($form, &$form_state) {
  // Take the existing repository and modify it with the given form values,
  // or construct a new one altogether.
  if (isset($form['#repository'])) {
    $repository = $form['#repository'];
    $repository->name                 = $form_state['values']['repo_name'];
    $repository->root                 = $form_state['values']['root'];
    $repository->update_method        = $form_state['values']['update_method'];
    $repository->plugins              = array(
      'author_mapper' => $form_state['values']['author_mapper'],
      'committer_mapper' => $form_state['values']['committer_mapper'],
      'auth_handler' => $form_state['values']['auth_handler'],
      'webviewer_url_handler' => $form_state['values']['url_handler'],
    );
  }
  else {
    $backends = versioncontrol_get_backends();
    $repository = array(
      'name' => $form_state['values']['repo_name'],
      'vcs' => $form['#vcs'],
      'root' => $form_state['values']['root'],
      'update_method' => $form_state['values']['update_method'],
      'backend' => $backends[$form['#vcs']],
      'plugins' => array(
        'author_mapper' => $form_state['values']['author_mapper'],
        'committer_mapper' => $form_state['values']['committer_mapper'],
        'auth_handler' => $form_state['values']['auth_handler'],
        'webviewer_url_handler' => $form_state['values']['url_handler'],
      ),
    );
    $repository = $backends[$form['#vcs']]->buildEntity('repo', $repository);
  }

  if (!empty($form_state['values']['base_url'])) {
    $repository->data['webviewer_base_url'] = $form_state['values']['base_url'];
  }

  // Let other modules alter the repository array.
  foreach (module_implements('versioncontrol_repository_submit') as $module) {
    $function = $module .'_versioncontrol_repository_submit';
    $function($repository, $form, $form_state);
  }

  if (!is_null($repository->repo_id)) {
    $repository->update();
    drupal_set_message(t('The %repository repository has been updated.', array('%repository' => $repository->name)));
  }
  else {
    $repository = $repository->insert();
    drupal_set_message(t('The %repository repository has been added.', array('%repository' => $repository->name)));
  }

  $form_state['redirect'] = 'admin/content/versioncontrol-repositories';
}


/**
 * Form callback for 'admin/content/versioncontrol-repositories/delete/%versioncontrol_repository':
 * Provide a form to confirm deletion of a repository.
 */
function versioncontrol_admin_repository_delete_confirm(&$form_state, $repository) {
  $form = array();
  $form['#repo_id'] = $repository->repo_id;

  $form = confirm_form($form,
    t('Are you sure you want to delete %name?', array('%name' => $repository->name)),
    !empty($_GET['destination']) ? $_GET['destination'] : 'admin/content/versioncontrol-repositories',
    t('Mind that by deleting the repository, all associated data such as commits and account associations will be deleted as well.'),
    t('Delete'), t('Cancel')
  );
  return $form;
}

/**
 * Delete the repository when the confirmation form is submitted.
 */
function versioncontrol_admin_repository_delete_confirm_submit($form, &$form_state) {
  $repository = versioncontrol_repository_load($form['#repo_id']);
  $repository->delete();
  drupal_set_message(t('The %repository repository has been deleted.', array(
    '%repository' => $repository->name,
  )));
  $form_state['redirect'] = 'admin/content/versioncontrol-repositories';
}

function versioncontrol_admin_repository_clearlock_confirm(&$form_state, $repository) {
  $form_state['repo_id'] = $repository->repo_id;
  $form_state['backend'] = $repository->vcs;

  $question = t('Are you sure you want to clear the lock on this repository?');
  $path = 'admin/content/versioncontrol-repositories';
  $description = t('Locks protect repositories while log fetches are being run. Breaking this lock could result in multiple fetches running in parallel, which will cause irrecoverable data inconsistencies.');
  $yes = t('Clear Lock');
  $no = t('Cancel');

  return confirm_form(array(), $question, $path, $description, $yes, $no);
}

function versioncontrol_admin_repository_clearlock_confirm_submit($form, &$form_state) {
  $backend = versioncontrol_get_backends($form_state['backend']);
  $repository = $backend->loadEntity('repo', array($form_state['repo_id']));

  $repository->locked = 0;
  $repository->update();
  $form_state['redirect'] = 'admin/content/versioncontrol-repositories';
}

function versioncontrol_admin_repository_fetch_confirm(&$form_state, $repository) {
  $form_state['repo_id'] = $repository->repo_id;
  $form_state['backend'] = $repository->vcs;

  $question = t('Are you sure you want to fetch logs from this repository?');
  $path = 'admin/content/versioncontrol-repositories';
  $description = t('Manually triggering a log fetch from this repository will attempt to fully synchronize database-stored data with all the latest changes in the repository. It <em>should</em> be non-destructive.');
  $yes = t('Fetch logs');
  $no = t('Cancel');

  return confirm_form(array(), $question, $path, $description, $yes, $no);
}

function versioncontrol_admin_repository_fetch_confirm_submit($form, &$form_state) {
  $backend = versioncontrol_get_backends($form_state['backend']);
  $repository = $backend->loadEntity('repo', array($form_state['repo_id']));

  $repository->fetchLogs();
  $form_state['redirect'] = 'admin/content/versioncontrol-repositories';
}
