<?php

/**
 * @file
 * Code for the admin pages of the govCMS CKAN module.
 */

/**
 * Create settings form for govCMS CKAN.
 *
 * @return array
 *   Form API definition.
 */
function govcms_ckan_settings_form() {

  $form = array();

  $form['govcms_ckan_endpoint_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Endpoint Url'),
    '#description' => t('Specify the endpoint url. Example https://data.gov.au (please note no trailing slash)'),
    '#weight' => 0,
    '#size' => 100,
    '#required' => TRUE,
    '#default_value' => variable_get('govcms_ckan_endpoint_url', ''),
  );

  $form['govcms_ckan_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Api Key'),
    '#description' => t('Specify the API key.'),
    '#weight' => 1,
    '#size' => 100,
    '#default_value' => variable_get('govcms_ckan_api_key', ''),
  );

  $form['govcms_ckan_auth_header'] = array(
    '#type' => 'textfield',
    '#title' => t('Authorisation header'),
    '#description' => t('Optionally override the HTTP authorisation header if required.'),
    '#weight' => 2,
    '#size' => 100,
    '#required' => FALSE,
    '#default_value' => variable_get('govcms_ckan_auth_header', ''),
  );

  $form['#validate'][] = 'govcms_ckan_settings_form_validate';

  return system_settings_form($form);
}

/**
 * Form validate handler for govCMS CKAN settings form.
 *
 * @see govcms_ckan_settings_form()
 */
function govcms_ckan_settings_form_validate($form, &$form_state) {
  // If an API key is in use, enforce https.
  if (!empty($form_state['values']['govcms_ckan_api_key'])) {
    if (file_uri_scheme($form_state['values']['govcms_ckan_endpoint_url']) != 'https') {
      form_set_error('govcms_ckan_endpoint_url', t('If using an API key, the endpoint url must use HTTPS.'));
    }
  }

  // Get a client instance.
  module_load_include('inc', 'govcms_ckan', 'src/GovCmsCkanClient');
  $client = new GovCmsCkanClient(
    $form_state['values']['govcms_ckan_endpoint_url'],
    $form_state['values']['govcms_ckan_api_key'],
    $form_state['values']['govcms_ckan_auth_header']);

  // Test the connection for a valid response.
  if (!empty($form_state['values']['govcms_ckan_api_key'])) {
    $response_code = $client->testConnection('action/dashboard_activity_list', array('limit' => 1));
  }
  else {
    $response_code = $client->testConnection('action/package_list', array('limit' => 1));
  }

  // If we don't get a 200 (success), we have problems connecting.
  if ($response_code != 200) {
    if ($response_code == 403) {
      form_set_error('govcms_ckan_api_key', t('API return "Not Authorised" please check your API key.'));
    }
    else {
      form_set_error('govcms_ckan_endpoint_url', t('Could not establish a connection to the endpoint. Error: @code',
        array('@code' => $response_code)));
    }
  }
}
