<?php
/**
 * @file
 * Handles CKAN Dataset Parsing.
 */

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
   * GovCmsCkanDatasetParser constructor.
   */
  public function __construct() {
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
  public function setTableAttributes($attributes) {
    $this->tableAttributes = array_merge($this->tableAttributes, $attributes);
    return $this;
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
      $tables[$group_key]['attributes'] = $this->tableAttributes;
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
    $cell = array('data' => $value, 'header' => TRUE, 'scope' => $scope);
    if (empty($value) && $scope == 'col') {
      $cell['data-placeholder'] = 'true';
    }
    return $cell;
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

    // We need to add the label field back to the keys for this formatter.
    $keys = $this->keys;
    array_unshift($keys, $this->labelKey);

    // Add keys as header.
    foreach ($keys as $key) {
      $table['header'][] = $this->labelKey == $key ? $this->createHeaderCell('col') : $key;
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

    // TODO: Should use an x axis label for this value.
    $table['header'] = array($this->createHeaderCell('col'));

    // Records form the columns.
    foreach ($records as $col_num => $record) {
      // As we use the first column for the x tick labels, data cols all move
      // over by one column.
      $data_col_num = $col_num + 1;

      // Append labelKey value to header.
      $table['header'][] = $record->{$this->labelKey};

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

}
