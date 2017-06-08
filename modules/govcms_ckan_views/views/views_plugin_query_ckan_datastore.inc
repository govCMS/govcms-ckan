<?php

/**
 * Class views_plugin_query_ckan_datastore
 *
 * Views query plugin that encapsulates calls to the API defined
 * in the CKAN module.
 */

class views_plugin_query_ckan_datastore extends views_plugin_query {

  /**
   * A list of filters to apply to the API request.
   * @var array
   */
  protected $filters = [];

//  protected $resources = ['23be1fc4-b6ef-4013-9102-c014c9d48711'];

  /**
   * A list of resource IDs we will be querying.
   * @var array
   */
  protected $resources = [
    'eb1e6be4-5b13-4feb-b28e-388bf7c26f93',
    'ce8bf129-9525-4353-a747-d89d8d4b5cc6',
  ];

  /**
   * Prepare the filters to be sent to the endpoint.
   *
   * @TODO this may need to change if the API supports different operators.
   */
  private function prepare_filters() {
    return json_encode($this->filters);
  }

  /**
   * Add a filter to the query.
   *
   * @param string $field
   *   A field to query on.
   * @param string $value
   *   The value to query for.
   *
   * @TODO: Investigate CKAN API as it may support operators to be parsed in
   * via the JSON string.
   */
  public function add_filter($field, $value) {
    $this->filters[$field] = $value;
  }

  /**
   * Resource ID/s should be set via the Resource ID filter.
   *
   * @param $id
   *   A CKAN resource ID.
   */
  public function add_resource($id) {
    $this->resources[] = $id;
  }

  /**
   * Send an API request to the views defined CKAN resource.
   *
   * A view can have multiple resources defined. This will allow the user
   * to query multiple datasets and merge the results into one graph.
   *
   * @TODO: Use the request class defined in CKAN module.
   * @TODO: Add filter to add resource IDs to the view as so not hard coding.
   */
  public function execute(&$view) {
    $view->result = [];

    foreach ($this->resources as $id) {
      $url = url('http://data.gov.au/api/3/action/datastore_search', [
        'query' => [
          'id' => $id,
          'q' => $this->prepare_filters(),
          'limit' => 100,
        ],
      ]);

      $response = drupal_http_request($url);
      $data = json_decode($response->data);

      $view->result[$id] = $data->result->records;
    }

  }

}
