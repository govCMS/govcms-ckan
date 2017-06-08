<?php

$plugin = [
  'name' => 'ckan_dataset',
  'base_table' => 'ckan_dataset', // Base table is used to register fields.
  'title' => t('CKAN Dataset'),
  'form_wizard_class' => [
    'file' => 'ckan_dataset_views_wizard.class.php',
    'class' => 'CkanDatasetViewsWizard',
  ],
];
