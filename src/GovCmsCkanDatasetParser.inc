<?php
/**
 * @file
 * Handles CKAN Dataset Parsing.
 */

require_once __DIR__ . '/GovCmsCkanRecord.inc';

/**
 * Defines the GovCmsCkanDatasetParser class.
 */
class GovCmsCkanDatasetParser {

  /**
   * Storage for result and records.
   */
  private $result;
  private $records;
  private $groupedRecords = array();

  /**
   * Result row keys are properties used in a single result row.
   *
   * @var array
   */
  private $keys = array();

  /**
   * The label key defines what key in $keys is the label for a result row.
   *
   * @var string
   */
  private $labelKey;

  /**
   * Group results based on this key.
   *
   * A common usage would be grouping into tables into years.
   *
   * @var string
   *   The key to group on.
   */
  private $groupKey;

  /**
   * X tick source for labels.
   *
   * This determines what gets used for the X tick labels or the labels (if any)
   * that populate the left column as opposed to the table headers (lines).
   *
   * @var string
   *   none, keys or values. @see setHeaderSource().
   */
  private $xtickSource = 'keys';

  /**
   * Format for a renderable array.
   *
   * This adds a # (hash) in front of the keys returned so it is in the format
   * suitable for a renderable array.
   */
  private $renderableArray = TRUE;

  /**
   * Attributes for returned table.
   */
  private $tableAttributes = array();

  /**
   * Attributes for a specific column.
   */
  private $columnAttributes = array();

  /**
   * Label replacements.
   */
  private $labelReplacements = array();

  /**
   * GovCmsCkanDatasetParser constructor.
   *
   * @param object $result
   *   Instantiate with result set, this can also be done later with setResult.
   */
  public function __construct($result = NULL) {
    if (!empty($result)) {
      $this->setResult($result);
    }
  }

  /**
   * Define the keys that contain the data in each result row.
   *
   * @param array $keys
   *   Array of properties in a result row, each should return a number apart
   *   from the labelKey which can return a string.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setKeys($keys = array()) {
    $this->keys = $keys;
    return $this;
  }

  /**
   * Re-orders the keys as per provided order array.
   *
   * @param array $order
   *   An array keyed by the key (original) name, the value for each should be
   *   an array containing a weight key. The lower the weight the higher it
   *   appears in the list. If no weight found, default order is used.
   * @param string $weight_key
   *   The key that contains the weight.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setKeysOrder($order = array(), $weight_key = 'weight') {
    $items = array();
    $new_order = array();
    // Give each key a weight either the original pos or overridden value.
    $i = 0;
    foreach ($this->keys as $pos => $key) {
      $weight = isset($order[$key][$weight_key]) ? $order[$key][$weight_key] : $i;
      $items[] = array('key' => $key, 'weight' => $weight);
      $i++;
    }
    // Sort and create a new array of keys in the correct order.
    uasort($items, array($this, 'sortWeight'));
    foreach ($items as $item) {
      $new_order[$item['key']] = $item['key'];
    }
    $this->keys = $new_order;
    return $this;
  }

  /**
   * Define which key in $keys is the property that returns the label string.
   *
   * @param string $label_key
   *   In a single result row, $row->{$labelKey} should return the label value.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setLabelKey($label_key) {
    $this->labelKey = $label_key;
    return $this;
  }

  /**
   * Define what will form the header (data labels).
   *
   * @param string $source
   *   Otions:
   *   - none: This will result in the x ticks being numbers starting at 0,
   *   incrementing by one at each tick and no left label column in the table.
   *   - keys: The keys provided will form the x ticks (left column labels)
   *   and the legend (table header) will be formed using the labelKey property.
   *   - values: The labelKey property in each result row will form the x tick
   *   (left column labels) and the keys will form the legend (table header).
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setHeaderSource($source = 'keys') {
    $this->xtickSource = $source;
    return $this;
  }

  /**
   * Set the result set.
   *
   * @param object $result
   *   A CKAN result object, should contain the property 'records'.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setResult($result) {
    $this->result = $result;
    $this->records = $result->records;
    $this->cleanseKeys();
    return $this;
  }

  /**
   * Add a grouping key.
   *
   * @param string $group_key
   *   A key that will split into multiple tables.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setGroupKey($group_key = NULL) {
    $this->groupKey = $group_key;
    return $this;
  }

  /**
   * Set if should be formatted for a renderable array.
   *
   * @param bool $renderable_array
   *   Should add a hash in front of returned keys.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setRenderableArray($renderable_array = TRUE) {
    $this->renderableArray = (bool) $renderable_array;
    return $this;
  }

  /**
   * Add attributes to the table. Can be reused as attributes are merged.
   *
   * @param array $attributes
   *   Attributes to add to the table.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setTableAttributes($attributes = array()) {
    $this->tableAttributes = array_merge($this->tableAttributes, $attributes);
    return $this;
  }

  /**
   * Adds attributes to the column headers.
   *
   * @param array $column_attributes
   *   An array keyed by the column/td value with its value being an array of
   *   attributes to be added to that td.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setColumnAttributes($column_attributes = array()) {
    $this->columnAttributes = $column_attributes;
    return $this;
  }

  /**
   * Adds label replacements.
   *
   * @param array $replacements
   *   An array of replacements keyed by the original label and the value is
   *   the replacement value.
   *
   * @return $this
   *   Return self for chaining.
   */
  public function setLabelReplacements($replacements = array()) {
    $this->labelReplacements = array_merge($this->labelReplacements, $replacements);
    return $this;
  }

  /**
   * Normalises attributes so all keys are lowercase and removes empty attrs.
   *
   * @return array
   *   An array of attributes.
   */
  public function normaliseAttributes($attributes) {
    $attrs = array();
    foreach ($attributes as $key => $value) {
      if ($value != '') {
        $attrs[strtolower($key)] = $value;
      }
    }
    return $attrs;
  }

  /**
   * Return the unique labels for a given key.
   *
   * @param string $key
   *   The key to filter by.
   *
   * @return array
   *   An array of keys
   */
  public function getUniqueLabels($key = NULL) {
    $labels = array();
    if (!empty($this->records) && is_array($this->records) && !empty($key)) {
      foreach ($this->records as $record) {
        if ($record->{$key}) {
          $labels[$record->{$key}] = $record->{$key};
        }
      }
    }
    return $labels;
  }

  /**
   * Parse the result set into headers and rows suitable for theme_table.
   *
   * @return array
   *   An array of tables, each with a 'header' and 'rows' key suitable for use
   *   with theme_table.
   */
  public function parse() {
    // Require records and keys to parse.
    if (empty($this->records) || empty($this->keys)) {
      return FALSE;
    }

    // Parse the records into groups.
    $this->parseGroups();

    // Get the parsing callback.
    $parse_callback = $this->tickSourceCallback();

    // Create tables for each group.
    $tables = array();
    foreach ($this->groupedRecords as $group_key => $records) {
      // Add the rows and header based on the formatter required.
      $tables[$group_key] = call_user_func(array($this, $parse_callback), $records);

      // Add any additional attributes and a generic class.
      $this->tableAttributes['class'][] = 'govcms-ckan-table';
      $tables[$group_key]['attributes'] = $this->normaliseAttributes($this->tableAttributes);
    }

    // Format for renderable array if required.
    if ($this->renderableArray === TRUE) {
      foreach ($tables as $group_key => $table) {
        $renderable_table = array('#theme' => 'table');
        foreach ($table as $key => $value) {
          $renderable_table['#' . $key] = $value;
        }
        $tables[$group_key] = $renderable_table;
      }
    }

    // Return parsed tables.
    return $tables;
  }

  /**
   * Get the correct callback method based on the x tick source.
   *
   * @return string
   *   The callback name.
   */
  private function tickSourceCallback() {
    $callbacks = array(
      'none' => 'parseCallbackNone',
      'keys' => 'parseCallbackKeys',
      'values' => 'parseCallbackValues',
    );
    return $callbacks[$this->xtickSource];
  }

  /**
   * Parse the result set into groups based on groupKeys set.
   *
   * Populates $groupedRecords with an array of groups, even if no grouping used
   * will transform records into an array with a key of 0.
   */
  private function parseGroups() {
    // Clear grouped records each time this is called.
    $this->groupedRecords = array();
    // If we have a grouping key.
    if (!empty($this->groupKey)) {
      // Group records by that key.
      foreach ($this->records as $r => $record) {
        $this->groupedRecords[$record->{$this->groupKey}][] = $record;
      }
    }
    else {
      // No group key set, for consistency, add to array.
      $this->groupedRecords[] = $this->records;
    }
  }

  /**
   * A helper to return a header cell.
   *
   * @param string $scope
   *   The scope of the cell, row or col.
   * @param string $value
   *   The value of the cell. If omitted the table call will be empty (&nbsp;).
   *
   * @return array
   *   Array with the data property as the value and data-xLabel attr set. If a
   *   col is empty, eg. placeholder it will be marked as empty.
   */
  private function createHeaderCell($scope = 'row', $value = NULL) {
    $cell = array('data' => $this->parseLabel($value), 'header' => TRUE, 'scope' => $scope);
    if (empty($value) && $scope == 'col') {
      $cell['data-placeholder'] = 'true';
    }
    elseif ($scope == 'col' && !empty($this->columnAttributes[$value])) {
      // Add any additional column attributes.
      $cell = array_merge($cell, $this->normaliseAttributes($this->columnAttributes[$value]));
    }
    return $cell;
  }

  /**
   * A helper to cleanse a value by removing new lines.
   *
   * @param string $value
   *   The value to cleanse.
   *
   * @return string
   *   The cleansed value.
   */
  public function cleanseValue($value = NULL) {
    return str_replace(array("\r\n", "\n", "\r"), ' ', $value);
  }

  /**
   * A helper to cleanse all keys in the record set.
   */
  public function cleanseKeys() {
    foreach ($this->records as $i => $record) {
      $clean_record = array();
      foreach ((array) $record as $key => $value) {
        $clean_record[$this->cleanseValue($key)] = $value;
      }
      $this->records[$i] = (object) $clean_record;
    }
  }

  /**
   * Applies any label replacements to label values.
   *
   * @param string $value
   *   The original label key.
   *
   * @return string
   *   If a replacement value exists return the replacement, else the original
   *   value.
   */
  private function parseLabel($value) {
    return !empty($this->labelReplacements[$value]) ? $this->labelReplacements[$value] : $value;
  }

  /**
   * Callback for uasort by weight.
   *
   * Exact copy of drupal_sort_weight() but included here to ensure class has no
   * dependance on drupal.
   */
  private function sortWeight($a, $b) {
    $a_weight = (is_array($a) && isset($a['weight'])) ? $a['weight'] : 0;
    $b_weight = (is_array($b) && isset($b['weight'])) ? $b['weight'] : 0;
    if ($a_weight == $b_weight) {
      return 0;
    }
    return ($a_weight < $b_weight) ? -1 : 1;
  }

  /**
   * Parse assuming the x ticks will start at 0 and increment by 1.
   *
   * Keys will form the table header.
   *
   * @return array
   *   Table variables with header and rows set.
   */
  private function parseCallbackNone($records) {
    $table = array();
    $records = $this->buildRecords($records);

    // Table header is simply the keys.
    $table['header'] = $this->keys;
    foreach ($records as $r => $record) {
      // Each key forms a column.
      foreach ($this->keys as $k => $key) {
        $table['rows'][$r][$k] = $record->{$key};
      }
    }

    return $table;
  }

  /**
   * Parse with keys forming header and labelKeys forming the x ticks.
   *
   * @return array
   *   Table variables with header and rows set.
   */
  private function parseCallbackKeys($records) {
    $table = array();
    $records = $this->buildRecords($records);

    // We need to add the label field back to the keys for this formatter.
    $keys = $this->keys;
    array_unshift($keys, $this->labelKey);

    // Add keys as header.
    foreach ($keys as $key) {
      $table['header'][] = $this->labelKey == $key ? $this->createHeaderCell('col') : $this->createHeaderCell('col', $key);
    }

    // For each record.
    foreach ($records as $record) {
      $row = array();
      // For each key.
      foreach ($keys as $key) {
        // If label key is disabled ignore the 'none' key and continue.
        if (empty($key)) {
          $row[] = NULL;
          continue;
        }

        // If it is a label key, it forms an x tick cell, otherwise data.
        $row[] = $this->labelKey == $key ? $this->createHeaderCell('row', $record->{$key}) : $record->{$key};
      }
      // Add to rows array.
      $table['rows'][] = $row;
    }

    return $table;
  }

  /**
   * Parse with labelKey forming header and keys forming the x ticks.
   *
   * @return array
   *   Table variables with header and rows set.
   */
  private function parseCallbackValues($records) {
    $table = array();
    $records = $this->buildRecords($records);

    // TODO: Should use an x axis label for this value.
    $table['header'] = array($this->createHeaderCell('col'));

    // Records form the columns.
    foreach ($records as $col_num => $record) {
      // As we use the first column for the x tick labels, data cols all move
      // over by one column.
      $data_col_num = $col_num + 1;

      // Append labelKey value to header.
      $table['header'][] = $this->createHeaderCell('col', $record->{$this->labelKey});

      // Make the rows from the keys.
      foreach ($this->keys as $row_num => $key) {

        // Add all rows that are not the label key.
        if ($this->labelKey != $key) {

          // Keys form the xLabel columns.
          if ($col_num == 0) {
            $table['rows'][$row_num][0] = $this->createHeaderCell('row', $key);
          }

          // Add the data to the correct row/column.
          $table['rows'][$row_num][$data_col_num] = $record->{$key};
        }
      }
    }

    return $table;
  }

  /**
   * Build record objects from a given dataset.
   *
   * @param array $records
   *   The resultant dataset.
   *
   * @return array
   *   An array of GovCmsCkanRecords.
   */
  public function buildRecords(array $records = array()) {
    foreach ($records as &$record) {
      $record = get_class($record) === 'GovCmsCkanRecord' ? $record : new GovCmsCkanRecord($record);
    }
    return $records;
  }

}
