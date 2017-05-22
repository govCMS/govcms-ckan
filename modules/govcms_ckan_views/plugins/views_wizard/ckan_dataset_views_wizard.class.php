<?php


class CkanDatasetViewsWizard extends ViewsUiBaseViewsWizard {

  /**
   * Override the build_form.
   *
   * Removes pager options from the wizard as these are not relevant.
   *
   * @TODO: Look at implementing pagination to this view type.
   */
  public function build_form($form, &$form_state) {
    $form = parent::build_form($form, $form_state);

    unset($form['displays']['page']['items_per_page']);
    unset($form['displays']['page']['options']['pager']);

    return $form;
  }

  /**
   * Override the build_form_style so we can specify which plugins to show.
   *
   * Relies on Style plugins being defined with a 'remote' type.
   *
   * @see parent::build_form_style
   * @see views_fetch_plugin_names
   * @see govcms_ckan_views_plugins
   */
  protected function build_form_style(&$form, &$form_state, $type) {
    // Potentially reconfigure the style plugins as 'ckan' as opposed to using
    // 'remote'.
    $style_options = views_fetch_plugin_names('style', 'remote', array($this->base_table));

    $style_form =& $form['displays'][$type]['options']['style'];
    $style_form['style_plugin']['#options'] = $style_options;
  }

  protected function default_display_options($form, $form_state) {
    $display_options = parent::default_display_options($form, $form_state);

    $display_options['style_plugin'] = 'ckan_visualisation';

    // Remove the default display option.
    unset($display_options['fields']);

    return $display_options;
  }

}
