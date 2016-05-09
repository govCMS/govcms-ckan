<?php
/**
 * @file
 * GovCmsCkanClient Class for fetching, caching and returning CKAN data.
 *
 * Basic Example Usage.
 * --------------------
 * $client = new GovCmsCkanClient($base_url, $api_key, $auth_header);
 * $client->get('action/package_show', array('id' => 'fd49dc83f86f'));
 *
 * Response Object.
 * ----------------
 * A standardised format for the object returned. It is cached locally and
 * contains the following properties:
 * - valid: (bool) If the API request was a success or failure.
 * - request_time (int) The time of the request to the API.
 * - code: (int) Response code from the request to the API http request.
 * - status: (string) Status message from the API http request.
 * - resource: (string) The requested resource.
 * - query: (array) The query params passed to the resource.
 * - data: (mixed) An object or array response from the API request.
 */

/**
 * Defines the GovCMS CKAN Client class.
 */
class GovCmsCkanClient {

  /**
   * API variables.
   */
  private $apiUrl;
  private $apiKey;
  private $authHeader;
  private $apiPath = '/api/%d/';
  private $apiVersion = 3;

  /**
   * Request variables.
   */
  private $resource;
  private $query;
  private $url;

  /**
   * Response variable, gets updated whenever get/fetch is called.
   */
  private $response;

  /**
   * Response object, contains the object to be returned.
   */
  private $responseObject;

  /**
   * Drupal cache table.
   *
   * TODO: Define custom cache bin.
   * http://www.exove.com/techblog/drupal-7-cache-bins-why-and-how-to/
   */
  private $cacheTable = 'cache';

  /**
   * How long to cache for if request was successful (in seconds).
   */
  private $cacheExpirySuccess = 2592000;

  /**
   * How long to cache for if request failed (in seconds).
   */
  private $cacheExpiryFail = 86400;

  /**
   * GovCmsCkanClient constructor.
   *
   * @param string $base_url
   *   The API base url for this endpoint.
   * @param string $api_key
   *   The API key for this endpoint.
   * @param int $api_version
   *   The version of the API to use.
   */
  public function __construct($base_url, $api_key = NULL, $auth_header = "Authorization", $api_version = 3) {
    $this->apiUrl = $base_url;
    $this->apiKey = $api_key;
    $this->authHeader = $auth_header;
    $this->apiVersion = $api_version;
  }

  /**
   * Test an endpoint is functional.
   *
   * @param string $resource
   *   The resource we are requesting.
   * @param array $query
   *   A key value pair of url paramaters.
   *
   * @return int
   *   The response code from the resource, 200 is success.
   */
  public function testConnection($resource, $query = array()) {
    // Build a test url.
    $test_url = url(
      $this->apiUrl . $this->apiPath() . $resource,
      array(
        'external' => TRUE,
        'query' => $query,
      ));

    // Add authentication.
    $options = array();
    if (!empty($this->apiKey)) {
      $options = array('headers' => array($this->authHeader => $this->apiKey));
    }

    // Return TRUE if response code is 200 (OK).
    $response = drupal_http_request($test_url, $options);
    return $response->code;
  }

  /**
   * Return the data.
   *
   * @param string $resource
   *   Resource path.
   * @param array $query
   *   A key pair array forming the url paramaters.
   *
   * @return mixed
   *   Parsed response.
   */
  public function get($resource, $query = array()) {
    // Prepare the variables for the url.
    $this->resource = $resource;
    $this->query = $query;

    // Fetch the response.
    $this->fetch();

    // Return the response object.
    return $this->responseObject;
  }

  /**
   * Fetch CKAN data.
   */
  private function fetch() {
    $this->url = url(
      $this->apiUrl . $this->apiPath() . $this->resource,
      array(
        'external' => TRUE,
        'query' => $this->query,
      ));

    // If no cache, we do a new request, parse and cache it.
    // If cache was present, the responseObject will be updated.
    if ($this->cacheDataGet() === FALSE) {
      // Make the request.
      $this->response = drupal_http_request($this->url, array(
        'headers' => array($this->authHeader => $this->apiKey),
      ));

      // Parse the response.
      $this->parseData();

      // Save to cache.
      $this->cacheDataSet();
    }
  }

  /**
   * Parse the raw CKAN data into a standardised object.
   */
  private function parseData() {
    // Build a generic response object.
    $this->responseObject = (object) array(
      'valid' => $this->isValidResponse(),
      'request_time' => time(),
      'code' => $this->response->code,
      'status' => $this->response->status_message,
      'url' => $this->url,
      'resource' => $this->resource,
      'query' => $this->query,
      'data' => (object) array(),
    );

    // Data only gets populated if we have a valid response.
    if ($this->responseObject->valid && isset($this->response->data)) {
      // TODO: Autodetect response format and handle errors if not JSON?
      $data = json_decode($this->response->data);
      $this->responseObject->data = $data->result;
      // There is a possibility that we get a 200 code but failed request.
      // @see http://docs.ckan.org/en/latest/api/#making-an-api-request
      $this->responseObject->valid = $data->success;
    }

    if (!$this->responseObject->valid) {
      $this->errorLogger();
    }
  }

  /**
   * Check the response is OK and valid.
   */
  private function isValidResponse() {
    return (isset($this->response->code) && $this->response->code == 200);
  }

  /**
   * Get the API path with the correct api version number.
   *
   * @return string
   *   The API path with version number included. Eg. '/api/3/'.
   */
  private function apiPath() {
    return sprintf($this->apiPath, $this->apiVersion);
  }

  /**
   * Handle errors.
   */
  private function errorLogger() {
    // Log to watchdog.
    watchdog('govcms_ckan_client',
      'Error requesting data from CKAN endpont: @url - Error @code - @status',
      array(
        '@url' => $this->url,
        '@code' => $this->response->code,
        '@status' => $this->response->status_message,
      ),
      WATCHDOG_ERROR);
  }

  /**
   * Populate the responseObject with the cached version if exists.
   *
   * @return bool
   *   TRUE if successful cache get, FALSE if not.
   */
  private function cacheDataGet() {
    $cache = cache_get($this->url);
    if (!empty($cache->data) && $cache->expire > time()) {
      $this->responseObject = $cache->data;
      return TRUE;
    }
    return FALSE;
  }

  /**
   * Save current responseObject to Drupal cache.
   *
   * We use the url for the cache key as that will be unique to the request.
   */
  private function cacheDataSet() {
    $expiry = $this->responseObject->valid ? $this->cacheExpirySuccess : $this->cacheExpiryFail;
    cache_set($this->url, $this->responseObject, $this->cacheTable, time() + $expiry);
  }

}
