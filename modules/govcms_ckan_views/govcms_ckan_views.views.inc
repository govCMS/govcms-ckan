<?php

/**
 * Implements hook_views_handlers().
 */
function govcms_ckan_views_views_handlers() {
  return [
    'info' => [
      'path' => drupal_get_path('module', 'govcms_ckan_views'),
    ],
    'handlers' => [],
  ];
}

/**
 * Implements hook_views_plugins().
 */
function govcms_ckan_views_views_plugins() {
  $base = ['ckan_dataset'];

  return [
    'module' => 'govcms_ckan_views',
    'query' => [
      'ckan_datastore' => [
        'title' => t('CKAN API Query'),
        'help' => t('Query handler to make API queries directly to a CKAN datastore'),
        'handler' => 'views_plugin_query_ckan_datastore',
      ],
    ],
    'style' => [
      'ckan_visualisation' => [
        'title' => t('CKAN Visualisation'),
        'help' => t('Display result data with CKAN visualisation plugins'),
        'handler' => 'views_plugin_style_ckan_visualisation',
        'help-topic' => 'style-ckan-visualisation',
        'uses row plugin' => FALSE,
        'uses row class' => FALSE,
        'uses options' => TRUE,
        'uses fields' => FALSE,
        'type' => 'remote',
        'base' => $base,
      ],
    ],
  ];
}

/**
 * Implements hook_views_data().
 */
function govcms_ckan_views_views_data() {
  $data = [];

  // Instruct views to use the defined query class for CKAN dataset views.
  $data['ckan_dataset']['table']['group'] = t('CKAN Datastore');
  $data['ckan_dataset']['table']['base'] = [
    'title' => t('CKAN datastore'),
    'help' => t('Query CKAN datastore API'),
    'query class' => 'ckan_datastore',
  ];

  // Add a filter to the view. This field will allow the view creator to specify
  // which API field the user will be submitting so we don't need to create a
  // specific filter for each type of field.
  //
  // @TODO: Potentially need to create generic "string", "int" filters if the
  // API supports different operators for each field.
  $data['ckan_dataset']['ckan_field'] = [
    'title' => t('CKAN Field'),
    'help' => t('A field that is present in the datastore'),
    'filter' => [
      'handler' => 'views_handler_filter_ckan',
    ],
  ];

  return $data;
}
