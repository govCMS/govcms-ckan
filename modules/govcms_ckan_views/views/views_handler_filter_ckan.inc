<?php

/**
 * Class views_handler_filter_ckan.
 *
 * Generic filter for CKAN visulisation views. This will allow you to specify
 * the field that you want to query with.
 */

class views_handler_filter_ckan extends views_handler_filter_string {

  /**
   * The field for the CKAN resource that this field uses.
   *
   * @var string
   */
  var $ckan_field_name = NULL;

  public function init(&$view, &$options) {
    parent::init($view, $options);
    $this->ckan_field_name = $this->options['ckan_field_name'];
  }

  /**
   * Define an option for the definition.
   *
   * Updates the option definiton for the filter to include the field name that
   * we will allow view creators to specify.
   *
   * @return array
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['ckan_field_name'] = array('default' => '');

    return $options;
  }

  /**
   * A list of operators that this filter can support.
   *
   * @TODO: Review the API docs to see if we can specify any other operators. A
   * brief review suggested that unless we use postgres mode we are limited to
   * contains queries.
   *
   * @return array
   */
  function operators() {
    return [
      'contains' => [
        'title' => t('Contains'),
        'short' => t('contains'),
        'method' => 'add_filter',
        'values' => 1,
      ],
    ];
  }

  /**
   * Add the filter to the views query.
   */
  function add_filter() {
    // @TODO: Fields are per resource an initial query to the CKAN endpoint will
    // reveal all fields that are available under keys. This could be done
    // when the form is loaded and then stored with this filter.
    $this->query->add_filter($this->ckan_field_name, $this->value);
  }

  /**
   * Add the CKAN field name field to the
   *
   * @see parent::value_form
   */
  public function value_form(&$form, &$form_state) {
    parent::value_form($form, $form_state);

    $form['ckan_field_name'] = [
      '#type' => 'textfield',
      '#title' => t('CKAN Field Name'),
      '#description' => t('The name of the field as it appears in the CKAN dataset.'),
      '#default_value' => $this->ckan_field_name,
    ];

    if (!empty($this->view->filter['ckan_resource_id'])) {
      $form['ckan_field_name']['#type'] = 'select';
      $form['ckan_field_name']['#options'] = $this->get_field_options();
    }
  }

  /**
   * Alter the exposed form when this filter is used.
   *
   * @see parent::exposed_form
   */
  public function exposed_form(&$form, &$form_state) {
    parent::exposed_form($form, $form_state);
    // Because we require the field name as input it will be appended to the
    // exposed filter form as well. Unsetting this here removes it, when the
    // field is added to the query Views will know which field is to be used.
    unset($form['ckan_field_name']);
  }

  /**
   * Make a request to the configured datasets and retrieve a list
   *
   * @return array
   *   As expected for
   */
  public function get_field_options() {
    // @TODO: Remote request if the 'ckan_resource_id' filter has been applied
    // to the view.
    return [];
  }

}
