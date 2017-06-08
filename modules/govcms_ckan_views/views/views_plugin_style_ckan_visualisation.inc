<?php

/**
 * Class views_plugin_style_ckan_visualisation
 *
 * Style plugin for views to integrate with CKAN visualisation plugins.
 */

class views_plugin_style_ckan_visualisation extends views_plugin_style {

  /**
   * @TODO: Update this so that the options form allow the user to choose which
   * visualisation to use when rendering the view.
   */

  /**
   * Ensure that uses_fields() always returns FALSE.
   *
   * This affects the Views UI by not allowing users to change how the display
   * behaves. All display options should be managed via code
   */
  public function uses_fields() {
    return FALSE;
  }

  /**
   * Render the results of a request to the CKAN API.
   *
   * This method should be used to handle grouping result sets together a view
   * should always group the response from the CKAN API via the resource ID.
   *
   * eg.
   *    {RESOURCE_ID} => [ ... ]
   *    {RESOURCE_ID} => [ ... ]
   *
   * @see views_plugin_style::render
   * @see views_plugin_style::render_grouping_sets
   *
   * @TODO: This should be rendered with the CKAN plugin so all JS is applied.
   */
  public function render() {
    $output = '';

    foreach ($this->view->result as $id => $result) {
      $row = reset($result);
      $headers = array_keys((array) $row);

      // @TODO: Instead of using theme functions use the render function defined
      // in the CKAN visualisation plugin.
      $output .= theme('table', [
        'header' => $headers,
        'rows' => json_decode(json_encode($result), TRUE),
      ]);
    }

    return $output;
  }
}
