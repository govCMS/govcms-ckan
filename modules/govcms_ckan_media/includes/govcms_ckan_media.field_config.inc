<?php
/**
 * @file
 * Defines the configuration widget field.
 */

/**
 * Implements hook_field_info().
 */
function govcms_ckan_media_field_info() {
  return array(
    'govcms_ckan_media_config' => array(
      'label' => t('CKAN Visualisation'),
      'description' => t('Configuration and display of a CKAN dataset.'),
      'default_widget' => 'govcms_ckan_media_config_widget',
      'default_formatter' => 'govcms_ckan_media_config_formatter',
      // Not working yet, @see https://www.drupal.org/node/680910.
      'entity_types' => array('file'),
    ),
  );
}

/**
 * Implements hook_field_instance_settings_form().
 */
function govcms_ckan_media_field_instance_settings_form($field, $instance) {
  // Placeholder for if and when we want to add additional field config.
  // Return settings form.
  return array();
}

/**
 * Implements hook_field_is_empty().
 */
function govcms_ckan_media_field_is_empty($item, $field) {
  return empty($item['config']) && (string) $item['config'] !== '0';
}

/**
 * Implements hook_field_widget_info().
 */
function govcms_ckan_media_field_widget_info() {
  return array(
    'govcms_ckan_media_config_widget' => array(
      'label' => t('Visualisation configuration'),
      'field types' => array('govcms_ckan_media_config'),
      'settings' => array(),
    ),
  );
}

/**
 * Implements hook_field_validate().
 */
function govcms_ckan_media_field_validate($entity_type, $entity, $field, $instance, $langcode, $items, &$errors) {
  // TODO: Let the plugin invoke validation.
}

/**
 * Implements hook_field_presave().
 *
 * Serialise configuration on field save.
 */
function govcms_ckan_media_field_presave($entity_type, $entity, $field, $instance, $langcode, &$items) {
  if ($field['type'] == 'govcms_ckan_media_config') {
    foreach ($items as $delta => $item) {
      // Only serialise if item['value'] is an array.
      if (is_array($item['config'])) {
        $items[$delta]['config'] = serialize($item['config']);
      }
    }
  }
}

/**
 * Implements hook_field_presave().
 *
 * Unserialise configuration on field load.
 */
function govcms_ckan_media_field_load($entity_type, $entities, $field, $instances, $langcode, &$items, $age) {
  if ($field['type'] == 'govcms_ckan_media_config') {
    foreach ($entities as $id => $entity) {
      foreach ($items[$id] as $delta => $item) {
        // Only unserialise if item['config'] is a string.
        if (is_string($item['config'])) {
          $items[$id][$delta]['config'] = unserialize($item['config']);
        }
      }
    }
  }
}

/**
 * Implements hook_field_widget_form().
 */
function govcms_ckan_media_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  // If not dealing with a config widget, exit here.
  // TODO: Also check we only dealing with a file_entity type.
  if ($instance['widget']['type'] != 'govcms_ckan_media_config_widget') {
    return $element;
  }

  // Things we use often.
  $field_name = $field['field_name'];
  $file = isset($form_state['file']) ? $form_state['file'] : (object) array();

  // Get the config values.
  $config = array();
  // As much of this is ajax loaded we check input first for current state.
  if (isset($form_state['input'][$field_name])) {
    $config = $form_state['input'][$field_name][$langcode][$delta]['config'];
  }
  elseif (isset($file->{$field_name}[$langcode][$delta])) {
    // Fallback to saved values.
    $config = $file->{$field_name}[$langcode][$delta]['config'];
  }

  // Base widget is a fieldset tree.
  $widget = $element + array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );

  // Ajax wrapper for including the visualisation settings.
  $vis_wrapper_id = 'visualisation-settings-' . $delta;

  // The visualisation selector. This will ajax load the configuration form for
  // the selected visualisation.
  $widget['visualisation'] = array(
    '#title' => t('Visualisation'),
    '#type' => 'select',
    '#default_value' => isset($config['visualisation']) ? $config['visualisation'] : NULL,
    '#required' => ($form_state['build_info']['form_id'] != 'field_ui_field_edit_form'),
    '#options' => _govcms_ckan_media_field_widget_visualisation_options(),
    '#ajax' => array(
      'callback' => 'govcms_ckan_media_field_widget_visualisation_form',
      'wrapper' => $vis_wrapper_id,
    ),
  );

  // The wrapper element for visualisation configuration.
  $widget['visualisation_config'] = array(
    '#type' => 'container',
    '#attributes' => array('id' => $vis_wrapper_id),
  );

  // If config says there is a configuration, load the form from the plugin.
  if (!empty($config['visualisation'])) {
    $config_form = govcms_ckan_get_plugin_config_form($config['visualisation'], $form, $form_state, $config['visualisation_config']);
    $widget['visualisation_config'] = array_merge($widget['visualisation_config'], $config_form);
  }

  $element['config'] = $widget;
  return $element;
}

/**
 * Ajax callback for changing visualisations.
 */
function govcms_ckan_media_field_widget_visualisation_form($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $parents = $form_state['triggering_element']['#parents'];
  array_pop($parents);
  $el = drupal_array_get_nested_value($form, $parents);
  return $el['visualisation_config'];
}


/************
 * Helpers.
 ************/

/**
 * Find the config field on a file entity and return its config.
 *
 * @param object $file
 *   File object.
 *
 * @return array
 *   The configuration, will be empty array if not found.
 */
function _govcms_ckan_media_get_visualisation_config($file) {
  // Only deal with ckan files.
  $scheme = file_uri_scheme($file->uri);
  if ($scheme !== 'ckan') {
    return array();
  }

  // Attempt to retrieve field name from static cache.
  $config_field = &drupal_static(__FUNCTION__ . $file->type, NULL);

  // Lookup config field name.
  if (empty($config_field)) {
    $fields_info = field_info_instances('file', $file->type);
    foreach ($fields_info as $field) {
      if (empty($config_field) && $field['widget']['type'] == 'govcms_ckan_media_config_widget') {
        $config_field = $field['field_name'];
      }
    }
  }

  // Return the first configuration. A visualisation shouldn't have multiples
  // deltas for config but if for some reason it will, change here.
  return !empty($config_field) && isset($file->{$config_field}[LANGUAGE_NONE]) ? $file->{$config_field}[LANGUAGE_NONE][0]['config'] : array();
}

/**
 * Get all the available visualisations as select options.
 *
 * @return array
 *   A key/value array of vis name/human name.
 */
function _govcms_ckan_media_field_widget_visualisation_options() {
  $plugins = govcms_ckan_get_all_plugins();
  $options = array();
  foreach ($plugins as $plugin) {
    $options[$plugin['name']] = $plugin['title'];
  }
  return $options;
}


/******************************
 * Config form default elements.
 ******************************/

/**
 * This helper provides the common elements for selecting the keys.
 *
 * This is implemented by the visualisation and merged into the form structure
 * for that plugin.  Most visualisations will use this.
 *
 * @param array $form
 *   Form array for the parent form.
 * @param array $form_state
 *   Current form state array from the parent form.
 * @param array $config
 *   The current configuration values.
 *
 * @return array
 *   Form structure for new elements.
 */
function govcms_ckan_media_visualisation_default_key_config($form, $form_state, $config = array(), $key_type = 'numeric') {
  $form = array();

  // We need a resource_id, however this file might not be saved yet, if it is
  // we can get it from the entity, otherwise, need to re-parse the embed code.
  if (isset($form_state['file'])) {
    $resource_id = $form_state['file']->resource_id;
  }
  else {
    $embed_code = $form_state['storage']['embed_code'];
    $handler = new MediaInternetGovCmsCkanHandler($embed_code);
    $uri = $handler->parse($embed_code);
    $resource_id = $handler->parseIdFromUri($uri);
  }

  // Make a request to obtain the fields.
  $client = govcms_ckan_client();
  $result = $client->get('action/datastore_search', array('id' => $resource_id));

  // If something went wrong with request we cannot continue.
  if ($result->valid === FALSE || !isset($result->data->fields)) {
    drupal_set_message(t('Unable to retrieve data for this package'), 'error');
    return $form;
  }

  // Get an instance of the parser.
  $parser = govcms_ckan_dataset_parser($result->data);

  // Loop over fields to build some lists for options.
  $options = array();
  foreach ($result->data->fields as $field) {
    // Cleanse the value (stripping new lines).
    $field->id = $parser->cleanseValue($field->id);
    // Ignore the ID field.
    if ($field->id == '_id') {
      continue;
    }
    // Group by field type.
    $options[$field->type][$field->id] = $field->id;
    // All fields.
    $options['all'][$field->id] = $field->id;
  }

  // Optional filter and query parameters.
  $form['ckan_filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('CKAN data filters'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // Default values for query string.
  if (isset($config['ckan_filters']['search'])) {
    $ckan_query_default = $config['ckan_filters']['search'];
  }
  else {
    $ckan_query_default = '';
  }

  $form['ckan_filters']['search'] = array(
    '#type' => 'textfield',
    '#title' => t('Full text query'),
    '#description' => t('Optionally query entire dataset for any string value.'),
    '#default_value' => $ckan_query_default,
  );

  // Default values for key/value filters.
  if (isset($config['ckan_filters']['filters'])) {
    $ckan_filters_default = $config['ckan_filters']['filters'];
  }
  else {
    $ckan_filters_default = '';
  }

  $form['ckan_filters']['filters'] = array(
    '#type' => 'textfield',
    '#title' => t('Filters'),
    '#description' => t('Filter on key/value dictionary. For example: {"code": "4000", "year": "2016"}.'),
    '#default_value' => $ckan_filters_default,
  );

  // Build the elements.
  $form['keys'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Keys'),
    '#options' => $options[$key_type],
    '#default_value' => $config['keys'],
    '#multiple' => TRUE,
    '#required' => TRUE,
    '#description' => t('What data to display in the visualisation.'),
  );

  $col_override_id = 'col-override-settings';
  $x_axis_grouping = isset($config['x_axis_grouping']) ? $config['x_axis_grouping'] : 'keys';
  $form['x_axis_grouping'] = array(
    '#type' => 'select',
    '#title' => t('X Axis Grouping'),
    '#options' => array(
      'keys' => t('Group by keys'),
      'values' => t('Group by label values'),
    ),
    '#default_value' => $x_axis_grouping,
    '#description' => t('Will the x axis be keys or values of the label key.'),
    '#ajax' => array(
      'callback' => 'govcms_ckan_media_visualisation_default_key_config_col_override_callback',
      'wrapper' => $col_override_id,
    ),
  );

  $form['labels'] = array(
    '#type' => 'select',
    '#title' => t('Label key'),
    '#options' => $options['all'],
    '#default_value' => $config['labels'],
    '#description' => t('What key contains the labels.'),
    '#ajax' => array(
      'callback' => 'govcms_ckan_media_visualisation_default_key_config_col_override_callback',
      'wrapper' => $col_override_id,
    ),
  );

  $form['split'] = array(
    '#type' => 'select',
    '#title' => t('Split field'),
    '#options' => $options['all'],
    '#default_value' => $config['split'],
    '#empty_option' => t('None'),
    '#description' => t('Optionally spit into multiple visualisations based on the value of this key.'),
  );

  $form['column_overrides'] = array(
    '#prefix' => '<div id="' . $col_override_id . '">',
    '#suffix' => '</div>',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#type' => 'fieldset',
    '#title' => t('Column overrides'),
    '#description' => t('Optionally override a style for a specific column, add one key|value per line and separate key value with a pipe.<br />Examples: <b>type|line</b> or <b>color|#000000</b> or <b>style|dashed</b>.'),
  );

  // Get the columns in use and make a text area for each to store its settings.
  $columns = $x_axis_grouping == 'keys' ? $options['all'] : $parser->getUniqueLabels($config['labels']);
  foreach ($columns as $column) {
    $form['column_overrides'][$column] = array(
      '#type' => 'textarea',
      '#rows' => 2,
      '#title' => filter_xss($column),
      '#default_value' => isset($config['column_overrides'][$column]) ? $config['column_overrides'][$column] : NULL,
    );
  }

  // This indicates to future parsers that these elements have been included.
  $form['key_defaults'] = array(
    '#type' => 'hidden',
    '#value' => 1,
  );

  return $form;
}

/**
 * Ajax callback for changing the label key or x grouping.
 */
function govcms_ckan_media_visualisation_default_key_config_col_override_callback($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
  $parents = $form_state['triggering_element']['#parents'];
  array_pop($parents);
  $el = drupal_array_get_nested_value($form, $parents);
  return $el['column_overrides'];
}

/**
 * This helper provides the common elements for graph display axis settings.
 *
 * Eg: labels, tick counts, etc.
 *
 * @param array $form
 *   Form array for the parent form.
 * @param array $form_state
 *   Current form state array from the parent form.
 * @param array $config
 *   The current configuration values.
 *
 * @return array
 *   Form structure for new elements.
 */
function govcms_ckan_media_visualisation_default_axis_config($form, $form_state, $config = array()) {
  $element = array();

  // Chart title.
  $element['show_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display chart title'),
    '#default_value' => $config['show_title'],
  );

  // Labels.
  $element['x_label'] = array(
    '#type' => 'textfield',
    '#title' => t('X axis label'),
    '#default_value' => $config['x_label'],
  );
  $element['y_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Y axis label'),
    '#default_value' => $config['y_label'],
  );

  // Advanced settings.
  $element['axis_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced axis settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  // Tick settings.
  $element['axis_settings']['x_tick_rotate'] = array(
    '#title' => t('X labels vertical'),
    '#type' => 'checkbox',
    '#default_value' => $config['axis_settings']['x_tick_rotate'],
    '#description' => t('Check to rotate the X axis labels 90 degrees.'),
  );
  $element['axis_settings']['x_tick_count'] = array(
    '#title' => t('X Tick Count'),
    '#type' => 'textfield',
    '#default_value' => $config['axis_settings']['x_tick_count'],
    '#element_validate' => array('element_validate_integer'),
    '#description' => t('The number of ticks on the X axis.'),
  );
  $element['axis_settings']['y_tick_count'] = array(
    '#title' => t('Y Tick Count'),
    '#type' => 'textfield',
    '#default_value' => $config['axis_settings']['y_tick_count'],
    '#element_validate' => array('element_validate_integer'),
    '#description' => t('The number of ticks on the Y axis.'),
  );

  // Limit label count.
  $element['axis_settings']['x_tick_cull'] = array(
    '#title' => t('X Label Count'),
    '#type' => 'textfield',
    '#default_value' => $config['axis_settings']['x_tick_cull'],
    '#element_validate' => array('element_validate_integer'),
    '#description' => t('The max number of labels on the Xaxis.'),
  );
  $element['axis_settings']['y_tick_cull'] = array(
    '#title' => t('Y Label Count'),
    '#type' => 'textfield',
    '#default_value' => $config['axis_settings']['y_tick_cull'],
    '#element_validate' => array('element_validate_integer'),
    '#description' => t('The max number of labels on the Y axis.'),
  );

  return $element;
}
