(function ($) {

  "use strict";

  /*
   * TableCharts jQuery plugin.
   * --------------------------
   * This is a class + jQuery plugin that handles turning data outputted in tabular
   * format into a standardised settings set ready to be used with charts.
   *
   * Usage.
   * ------
   * Call this on the table you want to create a chart for:
   * `$('.table-selector').tableCharts()`
   * Defaults are listed below in the tableChart class.
   *
   * Settings for the charts obtained from the dom, they can also optionally be passed to this class.
   * - Settings stored as attributes in the table element
   * -- data-type: The type of chart in use (eg. line, spline, bar, stacked, area, area-spline).
   * -- data-chart: The chart implementation to use (see below)
   * -- data-rotated: Default is false, change to true to rotate the table axis.
   * -- data-labels: Should labels be shown at each data point. Defaults to false
   * -- data-defaultView: Should the chart or table be displayed first. Defaults to 'chart'.
   * -- data-palette: A comma separated list of hex colours to be used for the palette.
   * -- data-grid: Grid lines to use: xy, x or y
   * -- data-hidePoints: Default is false, gives the ability to hide points. Only applies to line/spline.
   * -- data-pointSize: Define the point size, default is 2.5
   * -- data-showTitle : Should a title be rendered within the chart
   * -- data-title : The title string
   * -- data-dataOrder : Define the order of stacked datasets: desc, asc or null.
   * -- data-xLabel: The optional label to show on the X axis
   * -- data-yLabel: The optional label to show on the Y axis
   * -- data-xTickRotate: The angle to rotate X axis labels
   * -- data-xTickCount: The count of ticks on the X axis
   * -- data-yTickCount: The count of ticks on the Y axis
   * -- data-yTickValues: Override the tick values on the Y axis (comma separated values)
   * -- data-xTickValues: Override the tick values on the X axis (comma separated values)
   * -- data-yTickValueFormat: Format Y axis values in a number format.
   * -- data-xTickValueFormat: Format X axis values in a number format.
   * -- data-xDisableMultiLine: Disable wrapping of labels on the X axis.
   * -- data-xWidth: Specify a width for labels on the X axis.
   * -- data-numberFormatMinLength: Formatting of tick values only applies on numbers with a length gte to this.
   * -- data-xTickCull: The max count of labels on the X axis
   * -- data-xTickCentered: Are the x ticks centered above labels.
   * -- data-tickVisibility: Determines tick visibility. Options: show, hide-x, hide-y, hide-xy.
   * -- data-yMaxRound: The maximum amount of decimal places to allow in the Y axis ticks in auto mode.
   * -- data-yRounding: Enforce a number of of decimal places in the Y axis.
   * -- data-disableChartInteraction: Disable hover values on the chart. Default false.
   * -- data-disableLegendInteraction: Prevent hover/click defaults on legend. Default false
   * -- data-barWidth: Set the ratio for bar widths. If set to manual it will use barWidthOverride value.
   * -- data-barWidthOverride: Set the bar width (without a ratio), Requires barWidth to be 'manual'
   * -- data-chartPadding: Additional padding on edges of the chart. Expects object with: top, right, bottom, left.
   * -- data-chartSize: Override default chart width and height. Expects object with: width, height.
   * -- data-xPadding: Additional padding on edges of the axis. Expects object with: right, left.
   * -- data-yPadding: Additional padding on edges of the chart. Expects object with: top, bottom.
   * -- data-exportWidth: The width of the exported png. @see chartExport()
   * -- data-exportHeight: The height of the exported png. @see chartExport()
   * - Table headings (th) is used as the label and the following attributes can be used
   * -- data-color: Hex colour, alternative to using palette on the table element.
   * -- data-style: The style for the line (dashed, solid)
   * -- data-type: Override the chart type for a specific column of data.
   * - Each column forms a data set, Table headings can be strings but table values must be Ints.
   * -- If a tbody row has a th (scope=row), this will be used to form the x axis tick labels.
   * -- To ignore a thead th (eg placeholder for a label col) use the data-placeholder=true attr.
   *
   * Chart Implementations.
   * ----------------------
   * Chart implementations are used to turn the settings from tableChart into a chart.
   * They are abstracted from tableChart to allow flexibility for different chart types.
   *
   * To add a new chart implementation, add a function to the tableChartsChart object,
   * it will get passed the settings parsed by tableChart.
   *
   * c3js implementation is included in this file. @see tableChartsChart.c3js
   */

  /*
   * Storage for all the charts on the page.
   */
  var tableCharts = [];

  /*
   * Storage for the chart classes available.
   */
  window.tableChartsChart = window.tableChartsChart || {};

  /**
   * tableChart class.
   *
   * Handles the parsing and dom transformation of a table and hands settings over to
   * the selected tableChartsChart class to render the chart.
   *
   * @param dom
   *   The dom for a single table.
   */
  var TableChart = function(dom, settings) {
    var self = this;

    /*
     * Available options and defaults.
     */
    self.defaults = {
      // The table element dom.
      dom: dom,
      $dom: $(dom),
      // A unique Id for this chart.
      chartId: 0,
      // The dom Id for the chart element.
      chartDomId: 'table-chart-0',
      // The type of chart.
      type: 'line',
      // Type override with the key/type.
      types: {},
      // The tableChartChart class used to create the chart.
      chart: 'c3js',
      // Chart settings.
      rotated: false,
      palette: [],
      paletteOverride: {},
      labels: false,
      styles: [],
      disabledLegends: [],
      dataClasses: {},
      dataOrder: null,
      grid: null,
      gridLines: null,
      showTitle: false,
      title: 'chart',
      hidePoints: false,
      pointSize: 2.5,
      xLabel: null,
      yLabel: null,
      xTickRotate: 0,
      xTickCount: null,
      yTickCount: null,
      yTickValues: null,
      xTickValues: null,
      yTickValueFormat: null,
      xTickValueFormat: null,
      xTickType: null,
      xDateFormat: {},
      xDisableMultiLine: false,
      xWidth: null,
      numberFormatMinLength: 5,
      xTickCull: false,
      xTickCentered: true,
      xAxisLabelPos: 'inner-right',
      yAxisLabelPos: 'inner-top',
      tickVisibility: 'show',
      stacked: false,
      exportWidth: '',
      exportHeight: '',
      exportStylesheets: [],
      chartPadding: {},
      chartSize: {},
      xPadding: {},
      yPadding: {},
      disableChartInteraction: false,
      disableLegendInteraction: false,
      areaOpacity: 20,
      yMaxRound: 4,
      yRounding: null,
      barWidth: 0.5,
      barWidthOverride: 'auto',
      // The data for the chart.
      columns: [],
      data: {},
      group: [],
      // The labels to show on the x axis ticks.
      xLabels: ['x'],
      // Data attributes automatically parsed from the table element.
      dataAttributes: ['type', 'rotated', 'labels', 'defaultView', 'grid', 'xLabel', 'yLabel', 'xTickRotate',
        'xTickCount', 'yTickCount', 'yTickValues', 'xTickValues', 'yTickValueFormat', 'xTickValueFormat', 'xTickCull',
        'stacked', 'exportWidth', 'exportHeight', 'areaOpacity', 'xTickCentered', 'barWidth', 'yMaxRound', 'showTitle',
        'title', 'hidePoints', 'pointSize', 'xAxisLabelPos', 'yAxisLabelPos', 'gridLines', 'disableChartInteraction',
        'yTickValueRound', 'disableLegendInteraction', 'numberFormatMinLength', 'tickVisibility', 'chartPadding',
        'barWidthOverride', 'xPadding', 'yPadding', 'yRounding', 'dataOrder', 'xTickType', 'xDateFormat',
        'xDisableMultiLine', 'xWidth', 'chartSize'],
      // Chart views determine what is displaying chart vs table.
      chartViewName: 'chart',
      tableViewName: 'table',
      // defaultView must be either chartViewName or tableViewName.
      defaultView: 'chart',
      // The text for the toggle button {view} gets replaced with the view name.
      toggleText: 'Show {view}',
      // Component prefix used for dom classes and ids.
      component: 'table-chart',
      // Chart Initialized callback
      chartInitCallback: function () {
        self.triggerEvent('init:chart');
      }
    };

    // Settings start with defaults and extended by options passed to the constructor.
    self.settings = $.extend(self.defaults, settings);

    // Store the current view
    self.currentView = self.settings.defaultView;

    // Define the chartDomId based on the chartId.
    self.settings.chartDomId = self.settings.component + '-' + self.settings.chartId;

    /*
     * Parse settings from the table attributes.
     * @see settings.dataAttributes for what gets parsed.
     */
    self.parseSettings = function () {
      var val = null;

      // Available settings are found in the dataAttributes setting. We loop through
      // each of those and if not empty, override the settings.
      $(self.settings.dataAttributes).each(function (i, attr) {
        val = self.settings.$dom.data(attr.toLowerCase());
        if (val !== undefined && val !== null && val !== '') {
          self.settings[attr] = val;
        }
      });

      // Palette gets transformed into an array (may be overridden by table headings).
      if (self.settings.$dom.data('palette') !== undefined && self.settings.$dom.data('palette') !== '') {
        self.settings.palette = self.settings.$dom.data('palette').replace(' ', '').split(',');
      }

      // Return self for chaining.
      return self;
    };

    /*
     * Parse additional settings from table headings.
     *
     * @param $cell
     *   The cell jQuery object.
     * @param col
     *   The column number.
     *
     * @return string
     *   The value for this cell.
     */
    self.parseTableHeading = function ($cell, col) {
      // The content of the TH forms the value/key.
      var value = $cell.html();

      // Override colour for this data column.
      if ($cell.data('color') !== undefined) {
        self.settings.paletteOverride[value] = $cell.data('color');
      }

      // Currently only style option is dashed and will only work with line.
      // @see http://c3js.org/samples/simple_regions.html
      if ($cell.data('style') !== undefined && $cell.data('style') === 'dashed') {
        self.settings.styles.push({set: value, style: $cell.data('style')});
      }

      // Allows a column/heading to define its graph type, overriding the default.
      if ($cell.data('type') !== undefined) {
        self.settings.types[value] = $cell.data('type');
      }

      // Allows a column/heading to hide legends.
      if ($cell.data('legend') !== undefined && $cell.data('legend') === 'hide') {
        self.settings.disabledLegends.push(value);
      }

      // Allows a column/heading applying classes to a dataset.
      if ($cell.data('class') !== undefined) {
        self.settings.dataClasses[value] = $cell.data('class');
      }

      // Create a group of headings (used for stacking).
      self.settings.group.push(value);

      // Return the value for this cell.
      return value;
    };

    /*
     * Parse the data from the table into an array suitable for c3js/d3js.
     *
     * NOTE: Assumes that all tables have a TH and this is the label for
     * that data set. All other values in that column are assumed to be integers.
     */
    self.parseData = function () {
      var columns = [], val, $cell, col, isHeader;

      // On each row.
      $('tr', self.settings.$dom).each(function (r, row) {
        col = 0;
        isHeader = $(row).parent().is('thead');

        // On each cell.
        $('th,td', row).each(function (c, cell) {
          $cell = $(cell);

          // Don't process placeholder header cells (th used for row headers).
          if (isHeader && $cell.data('placeholder') === true) {
            return;
          }

          // If a row header, the parser deals with it and we skip to the next cell.
          if ($cell.is('th') && !isHeader) {
            self.settings.xLabels.push($cell.html());
            return;
          }

          // Create our data set if doesn't exists yet.
          columns[col] = columns[col] || [];

          // If dealing with the table headers.
          val = isHeader === true ? self.parseTableHeading($cell, c) : parseFloat($cell.html());

          // Add the rows to the correct data set.
          columns[col].push(val);
          col++;

        });

      });

      // Add parsed data rows to settings.
      self.settings.columns = columns;

      // Return self for chaining.
      return self;
    };

    /*
     * Helper to create toggle button text.
     *
     * @return string
     *   The text for a toggle button based on the state.
     */
    self.toggleButtonText = function (view) {
      var s = self.settings;
      if (view === undefined) {
        // If no view specified, use the opposite of what the defaultView is.
        view = s.defaultView === s.chartViewName ? s.tableViewName : s.chartViewName;
      }
      return s.toggleText.replace('{view}', view);
    };

    /*
     * Toggle visibility of the chart and table.
     */
    self.toggleView = function () {
      self.$chart.toggle();
      self.$tableWrapper.toggle();
      // Update button text.
      self.$toggle.html(self.toggleButtonText(self.currentView));
      // Update current view.
      self.currentView = self.currentView === self.settings.chartViewName ? self.settings.tableViewName : self.settings.chartViewName;

      // Return self for chaining.
      return self;
    };

    /*
     * Prepare the markup on the page for charts.
     */
    self.buildMarkup = function () {
      // Build our chart dom ID, and dom elements we'll need.
      self.$chart = $('<div>');
      self.$actions = $('<div>').addClass(self.settings.component + '--actions');
      self.$toggle = $('<button>');

      // Give the table a unique class and give it a wrapper.
      self.settings.$dom
        .addClass(self.settings.component + '--table')
        .wrap('<div class="' + self.settings.component + '--table-wrapper"></div>');

      // Store the wrapper as we use this to toggle visibility.
      self.$tableWrapper = self.settings.$dom.parent();

      // Add a toggle button after the table.
      self.$toggle.html(self.toggleButtonText())
        .addClass(self.settings.component + '--toggle')
        .click(self.toggleView)
        .appendTo(self.$actions);

      // Add actions to the dom.
      self.$actions.insertAfter(self.$tableWrapper);

      // Add chart placeholder to dom with a unique Id.
      self.$chart.attr('id', self.settings.chartDomId)
        .addClass(self.settings.component + '--chart')
        .addClass(self.settings.component + '--chart--' + self.settings.type)
        .insertAfter(self.$tableWrapper);

      // Display only table or chart depending on defaultView.
      if (self.settings.defaultView === self.settings.chartViewName) {
        self.$tableWrapper.hide();
      } else {
        self.$chart.hide();
      }

      self
        // Now the markup is ready, build the chart.
        .buildChart()
        // Add download buttons. TODO: Move elsewhere.
        .addDownloadButtons();

      // Return self for chaining.
      return self;
    };

    /*
     * Build the chart based on the chart setting.
     */
    self.buildChart = function () {
      // If a chart implementation exists, call it and pass the settings.
      if (typeof tableChartsChart[self.settings.chart] === 'function') {
        tableChartsChart[self.settings.chart](self.settings);
      } else {
        // No implementation found.
        self.$chart.html('No chart implementation found for ' + self.settings.chart);
      }

      // Return self for chaining.
      return self;
    };

    /*
     * Add download buttons, requires $.chartExport()
     *
     * TODO: Move to a more appropriate place! Should not live in this class.
     */
    self.addDownloadButtons = function() {
      if (typeof $.fn.chartExport !== 'function') {
        return;
      }
      var buttonTypes = ['svg', 'png'];
      $(buttonTypes).each(function (i, format) {
        $('<button>')
          .html('Download as ' + format)
          .insertAfter(self.$toggle)
          .addClass(self.settings.component + '--download')
          .chartExport({
            format: format,
            svg: self.$chart,
            width: self.settings.exportWidth,
            height: self.settings.exportHeight,
            exportStylesheets: self.settings.exportStylesheets,
            filename: self.settings.title
          });
      });

      // Return self for chaining.
      return self;
    };

    /*
     * Trigger an event so other scripts can listen for changes.
     */
    self.triggerEvent = function (event) {
      $(window).trigger({type: 'tableCharts:' + event, el: self});

      // Return self for chaining.
      return self;
    };

    /*
     * Initialize the class.
     */
    self.init = function () {
      self
        .parseSettings()
        .parseData()
        .triggerEvent('init:parse')
        .buildMarkup()
        .triggerEvent('init:dom');
    };

    // Init on construct.
    self.init();

    // Return self for chaining.
    return self;
  };

  /**
   * jQuery plugin/function.
   */
  $.fn.tableCharts = function (settings) {
    window.tableCharts = window.tableCharts || [];
    settings = settings || {};
    return this.each(function (i, dom) {
      // Store all the charts on the page in tableCharts.
      // Each chart needs a unique ID for the page.
      settings.chartId = window.tableCharts.length + 1;
      window.tableCharts.push(
        new TableChart(dom, settings)
      );
    });
  };

})(jQuery);
