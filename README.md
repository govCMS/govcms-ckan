INTRODUCTION
============

govCMS CKAN provides integration with CKAN (http://ckan.org/). CKAN is a
powerful data management system that makes data accessible by providing tools to
streamline publishing, sharing, finding and using data.

Authors
-------
This module was created by [Jeremy Graham](https://github.com/jeremy-doghouse) @ [Doghouse Agency](http://doghouse.agency)
for [GovCMS](https://www.govcms.gov.au/) with lots of help and guidance from [Acquia](https://www.acquia.com/)
and [these contributors](https://github.com/govCMS/govcms-ckan/graphs/contributors).

Submodules
----------

govCMS CKAN Display
- Handles turning the tabular data into charts.
- Provides ability to download charts as svg/png.
- Provides visualisation plugins for common formats (eg. Bar, spline, line)

govCMS CKAN Display Examples
- Provides an examples page for viewing different chart types.

govCMS CKAN Media
- Provides media internet integration.
- Provides a CKAN file type and stream wrapper/handler.
- Visualisation configuration field/widget.
- Visualisation formatter/renderer.

GovCmsCkanClient Class
----------------------
This client class handles fetching, caching and returning data from a CKAN endpoint.

The CKAN base URL and API key are configured via the admin UI. When an endpoint request
is made it will first check if there is a local cached version and if not expired,
will return that. If no cache exists or expired a new request to the endpoint is performed
and cached.

Basic Usage of the class:
```
// Get a new instance of the class.
$client = govcms_ckan_client();

// Make a request.
$resource = 'action/package_show';
$query_params = array('id' => '1234');
$client->get($resource, $query_params);
```

This will make a request to the api which will look similar to this:
http://my.ckan.enpoint.baseurl/api/3/action/package_show?id=1234

Valid responses:

The response object should always contain a `valid` property to indicate if the data
returned is valid/usable. If FALSE there was either an error in the http request or
possibly the CKAN request. All failed requests will get added to watchdog.

Examples of resources:
action/package_show - metadata for a resource
action/datastore_search - records (data) for a resource

Wrappers/Helpers:
```
// Return a resource metadata.
$meta = govcms_ckan_client_request_meta($resource_id);

// Return the records for a resource.
$records = govcms_ckan_client_request_records($resource_id);
```

GovCmsCkanDatasetParser Class
-----------------------------
This parser class handles parsing the response records from the client class into table(s).

The parsed output is in a format for handing straight to theme_table either directly or by
using a renderable array. Data can be represented in multiple ways, and this class provides
setters to control how the data/table will be presented.

Basic usage of the class:
```
// Get a new instance of the class.
$parser = govcms_ckan_dataset_parser();

// Set the options.
$parser
  ->setResult($response_data)
  ->setKeys($keys)
  ->setLabelKey($label_key)
  ->setHeaderSource($x_axis_grouping)
  ->setTableAttributes($attributes)
  ->setGroupKey($group_key);
  ->parse();
```
GovCmsCkanDatasetParser.inc is well documented, see comments for what each setter does. You
can also look at some of the visualisations used in govcms_ckan_display for working examples.

Visualisation plugins
---------------------
Visualisations are cTools plugins, a module must implement the following for plugins to be registered:
```
function hook_ctools_plugin_directory($module, $plugin) {
  if ($module == 'govcms_ckan' && in_array($plugin, array_keys(govcms_ckan_ctools_plugin_type()))) {
    return 'plugins/' . $plugin;
  }
}
```
Visualisation plugins should live in the `plugins/visualisations` folder.

The best place to start with creating a new visualisation is by copying a visualisation from
`govcms_ckan_display/plugins/visualisation` to your own module/theme and the customise.

Each plugin file should contain the following structure:
```
$plugin = array(
  'title' => 'My visualisation title',
  'settings' => array(),
);
function MODULE_NAME_PLUGIN_NAME_view($file, $display, $config){
  // Return renderable array of visualisation.
}
function MODULE_NAME_PLUGIN_NAME_configure($plugin, $form, $form_state, $config){
  // Return form structure array for configuration elements.
}
```

Configuration helpers:

There is a few helpers that include common structure/elements for visualisation configure forms.
The intention is a plugin configuration form will include these as required.
```
// Get the key selection form elements.
$key_elements = govcms_ckan_media_visualisation_default_key_config($form, $form_state, $config);

// Get the axis configuraion elements.
$axis_elements = govcms_ckan_media_visualisation_default_axis_config($form, $form_state, $config);
```
