<?php
/**
 * @file
 * Standard way for accessing a CKAN record.
 */

/**
 * Class GovCmsCkanRecord
 *
 * @group govcms_ckan
 */
class GovCmsCkanRecord {

  /**
   * GovCmsCkanRecord constructor.
   *
   * Builds a CKAN response record from a CKAN request object.
   *
   * @param array $properties
   *   A list $key => $value pairs.
   */
  public function __construct($properties = []) {
    $properties = (array) $properties;
    foreach ($properties as $prop => $value) {
      $this->$prop = $value;
    }
  }

  /**
   * Provide access to properties on the object.
   *
   * @param string $property
   *   A property to access.
   * @param mixed $default
   *   A default value to provide if $property is undefined.
   *
   * @return mixed
   *   $property or $default.
   */
  public function get($property = '', $default = NULL) {
    return is_null($this->$property) ? $default : $this->$property;
  }

  /**
   * {@inheritdoc}
   */
  public function __get($property) {
    return isset($this->$property) ? $this->$property : NULL;
  }

}
