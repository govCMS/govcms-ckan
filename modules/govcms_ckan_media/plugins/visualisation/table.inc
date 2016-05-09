<?php
/**
 * @file
 * Basic table visualisation (default).
 */

$plugin = array(
  'title' => t('Table visualisation'),
  'ckan_filters' => array(
    'search' => NULL,
    'filters' => NULL,
    'column_overrides' => array(),
  ),
);

/**
 * Returns a renderable array that represents the block content.
 */
function govcms_ckan_media_table_view($file, $display, $config) {
  $element = array();
  $chart_class = 'ckan-table';
  $response = govcms_ckan_client_request_records($file->resource_id, $config['ckan_filters']['search'], $config['ckan_filters']['filters']);

  // If failure, provide error message.
  if ($response->valid !== FALSE) {

    // Setup our configuration.
    $keys = array_filter($config['keys']);
    $column_overrides = array_map('govcms_ckan_string_to_array', $config['column_overrides']);

    // Attributes for the table.
    $attributes = array(
      'class' => array($chart_class),
    );

    // Parse the data.
    $parser = govcms_ckan_dataset_parser($response->data);
    $element = $parser
      ->setKeys($keys)
      ->setLabelKey($config['labels'])
      ->setGroupKey($config['split'])
      ->setHeaderSource('keys')
      ->setTableAttributes($attributes)
      ->setColumnAttributes($column_overrides)
      ->parse();
  }

  return $element;
}

/**
 * Configure form callback.
 */
function govcms_ckan_media_table_configure($plugin, $form, $form_state, $config) {
  // Get default key elements.
  $config_form = govcms_ckan_media_visualisation_default_key_config($form, $form_state, $config, 'all');

  // Allow for an empty selection on label key.
  $config_form['labels']['#empty_option'] = t('None');

  // Return basic key configuration.
  return $config_form;
}
