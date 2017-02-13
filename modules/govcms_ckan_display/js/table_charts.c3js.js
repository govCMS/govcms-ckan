(function ($) {
  /*
   * This is the c3js implementation class for tableCharts.
   *
   * It is called by the tableCharts class and assumes data has been parsed from
   * the table and all applicable settings and data are present.
   *
   * @see tableCharts default settings for structure and available settings.
   *
   * @param settings
   *   The parsed settings from tableCharts.
   */

  // All tableCharts chart implementations should be a method of this object.
  window.tableChartsChart = window.tableChartsChart || {};

  // Store all instances of c3js generated charts by id.
  window.tableChartC3jsCharts = window.tableChartC3jsCharts || {};

  tableChartsChart.c3js = function (settings) {

    // Ensure library is loaded.
    if (typeof c3 === 'undefined') {
      alert('c3js library not found');
      return;
    }

    // Self instance.
    var self = this;

    // Store settings pased from parser.
    self.settings = settings;

    // Base options object that gets passed to c3js.
    self.options = {
      bindto: '#' + self.settings.chartDomId,
      color: {pattern: self.settings.palette},
      oninit: function () {
        self.postBuildCallback();
      },
      data: {},
      axis: {}
    };

    /*
     * Initialize the C3 Chart.
     */
    self.init = function () {
      // Parse all the settings.
      self
        .parseDataOptions()
        .parseAxisOptions()
        .parseGridOptions()
        .parsePointOptions()
        .parseBarOptions()
        .parseChartOptions();

      // Create chart.
      window.tableChartC3jsCharts[self.settings.chartDomId] = c3.generate(self.options);
    };

    /*
     * Parse data and data settings.
     */
    self.parseDataOptions = function () {
      var data = {};

      // Type of chart is stored in the data.
      data.type = self.settings.type;

      // Placeholder for the data columns.
      data.columns = [];

      // Stacked can be applied to most charts, the stack order used is
      // the column order.
      if (self.settings.stacked) {
        data.groups = [self.settings.group];
        data.order = self.settings.dataOrder;
      }

      // Apply styles (currently only works with lines and dashes)
      if (self.settings.styles.length) {
        data.regions = {};
        $(self.settings.styles).each(function (i, d){
          data.regions[d.set] = [{style: d.style}];
        });
      }

      // Add X axis labels.
      if (self.settings.xLabels.length > 1) {
        data.x = 'x';
        data.columns.push(self.settings.xLabels);
      }

      // Show labels on data points?
      data.labels = self.settings.labels;

      // Add any overrides to graph types based on the column.
      data.types = self.settings.types;
      data.colors = self.settings.paletteOverride;

      // Add the data columns.
      $(self.settings.columns).each(function (i, col) {
        data.columns.push(col);
      });

      // Add optional classes.
      data.classes = self.settings.dataClasses;

      // Add date input formatting if available and timeseries.
      if (self.settings.xTickType === 'timeseries' && self.settings.xDateFormat.input) {
        data.xFormat = self.settings.xDateFormat.input;
      }

      // Add to options.
      self.options.data = data;

      // Return self for chaining.
      return self;
    };

    /*
     * Parse the axis options.
     */
    self.parseAxisOptions = function () {
      // Define the axis settings.
      var axis = {
        rotated: self.settings.rotated,
        x: {label: {text: self.settings.xLabel}, tick: {}},
        y: {label: {text: self.settings.yLabel}, tick: {}}
      };

      // Define the tick rotation.
      if (self.settings.xTickRotate !== 0) {
        axis.x.tick.rotate = parseInt(self.settings.xTickRotate);
      }

      // Define the tick counts.
      if (self.settings.xTickCount) {
        axis.x.tick.count = parseInt(self.settings.xTickCount);
      }
      if (self.settings.yTickCount) {
        axis.y.tick.count = parseInt(self.settings.yTickCount);
      }

      // Override Y axis values if defined.
      if (self.settings.yTickValues) {
        axis.y.tick.values = self.settings.yTickValues.replace(' ', '').split(',');
      }

      // Override Y axis values if defined.
      if (self.settings.xTickValues) {
        axis.x.tick.values = self.settings.xTickValues.replace(' ', '').split(',');
      }

      // Define the tick label culling (max labels).
      if (self.settings.xTickCull !== false) {
        axis.x.tick.culling = {max: parseInt(self.settings.xTickCull)};
        // Round labels to whole numbers.
        axis.x.tick.format = function (x) {
          return Math.round(x);
        };
      }

      // Add date output formatting if available and timeseries.
      if (self.settings.xTickType === 'timeseries' && self.settings.xDateFormat.output) {
        axis.x.tick.format = self.settings.xDateFormat.output;
      }

      // Format Y axis ticks.
      axis.y.tick.format = function (y) {
        var value = self.maxRound(y);
        // Enforce Y rounding to specific decimal place.
        if (!!self.settings.yRounding) {
          value = value.toFixed(self.settings.yRounding);
        }
        // Apply number formatting to the tick value.
        if (self.settings.yTickValueFormat) {
          return self.formatNumber(value, self.settings.yTickValueFormat);
        }
        return value;
      };

      // Format X axis ticks, as this potentially could be categories we need
      // to check if value formatting has been explicitly set before using format().
      if (self.settings.xTickValueFormat && self.settings.xTickValueFormat !== '') {
        axis.x.tick.format = function (x) {
          //Apply number formatting to the tick value.
          return self.formatNumber(x, self.settings.xTickValueFormat);
        };
      }

      // Tick culling prevents this being a category axis.
      if (self.settings.xLabels.length > 1 && self.settings.xTickCull === false && !self.settings.xTickValues) {
        axis.x.type = 'category';
      }

      // Force the X axis to a specific type. This trumps auto setting above.
      if (self.settings.xTickType) {
        axis.x.type = self.settings.xTickType;
      }

      // X Axis tick centered.
      if (self.settings.xTickCentered) {
        axis.x.tick.centered = self.settings.xTickCentered;
      }

      // Set the label positions. If rotated we need to swap position settings.
      if (self.settings.xAxisLabelPos || (self.settings.rotated && self.settings.yAxisLabelPos)) {
        axis.x.label.position = (!self.settings.rotated ? self.settings.xAxisLabelPos : self.settings.yAxisLabelPos);
      }
      if (self.settings.yAxisLabelPos || (self.settings.rotated && self.settings.xAxisLabelPos)) {
        axis.y.label.position = (!self.settings.rotated ? self.settings.yAxisLabelPos : self.settings.xAxisLabelPos);
      }

      // X label disable wrapping.
      if (self.settings.xDisableMultiLine) {
        axis.x.tick.multiline = false;
      }

      // X label width.
      if (self.settings.xWidth) {
        axis.x.tick.width = self.settings.xWidth;
      }

      // X and Y axis padding.
      if (typeof self.settings.xPadding === "object") {
        axis.x.padding = self.settings.xPadding;
      }
      if (typeof self.settings.yPadding === "object") {
        axis.y.padding = self.settings.yPadding;
      }

      // Add to options.
      self.options.axis = axis;

      // Return self for chaining.
      return self;
    };

    /*
     * Parse grid options.
     */
    self.parseGridOptions = function () {
      var grid = {x: {lines: []}, y: {lines: []}};

      // Add default grid.
      switch (self.settings.grid) {
        case 'xy':
          grid.x.show = true;
          grid.y.show = true;
          break;
        case 'x':
          grid.x.show = true;
          break;
        case 'y':
          grid.y.show = true;
          break;
      }

      // Add additional grid lines.
      if (self.settings.gridLines) {
        $.each(self.settings.gridLines, function(i, line) {
          grid[line.axis].lines.push(line);
        });
      }

      self.options.grid = grid;

      // Return self for chaining.
      return self;
    };

    /*
     * Parse point options.
     */
    self.parsePointOptions = function () {
      var point = {};

      // Optionally hide points from line/spline charts.
      if (self.settings.hidePoints) {
        point.show = false;
      }

      // Optionally set the point size.
      if (self.settings.pointSize) {
        point.r = self.settings.pointSize;
      }

      self.options.point = point;

      // Return self for chaining.
      return self;
    };

    /*
     * Parse bar options.
     */
    self.parseBarOptions = function () {
      if (self.settings.type === 'bar') {
        if (self.settings.barWidth === 'manual') {
          // Define the width of the bar manually (not using a ratio).
          self.options.bar = {width: self.settings.barWidthOverride};
        } else {
          // Provide a width ratio for bars.
          self.options.bar = {width: {ratio: self.settings.barWidth}};
        }
      }

      // Return self for chaining.
      return self;
    };

    /*
     * Parse generic chart options.
     */
    self.parseChartOptions = function () {
      self.options.legend = {};

      // Add optional title.
      if (self.settings.showTitle) {
        self.options.title = {text: self.settings.title};
      }

      // Disable legend click/hover.
      if (self.settings.disableLegendInteraction) {
        self.options.legend.item = {
          onclick: function () {
            return false;
          },
          onmouseover: function () {
            this.api.revert();
            return false;
          }
        };
      }

      // Additional padding.
      if (self.settings.chartPadding) {
        self.options.padding = self.settings.chartPadding;
      }

      // Disable chart interaction.
      if (self.settings.disableChartInteraction) {
        self.options.interaction = {enabled: false};
      }

      // Hide specific dataset legends.
      if (self.settings.disabledLegends.length > 0) {
        self.options.legend.hide = self.settings.disabledLegends;
      }

      // Chart dimensions.
      if (self.settings.chartSize) {
        self.options.size = self.settings.chartSize;
      }

      // Return self for chaining.
      return self;
    };

    /*
     * Format a number with a separator at the 3n interval.
     */
    self.formatNumber = function (number, separator) {
      // Separate decimal places (if any).
      var nParts = number.toString().split('.');
      // Formatting only applies on a string length gte to numberFormatMinLength
      if (nParts[0].toString().length < self.settings.numberFormatMinLength) {
        return number;
      }
      // Add a separator at 3n the append back the decimals (if any).
      return nParts[0].toString().replace(/./g, function(c, i, a) {
        return i && c !== "." && ((a.length - i) % 3 === 0) ? separator + c : c;
      }) + (nParts[1] !== undefined ? '.' + nParts[1] : '');
    };

    /*
     * Apply max rounding to a number based on yRound (default 4) decimal places.
     * If decimal values are useless (all zeros) they will be removed.
     *
     * Eg. '4.000' will output as '4', and '4.123456' will output as '4.1234'.
     */
    self.maxRound = function(number) {
      var places = Math.pow(10, parseInt(self.settings.yMaxRound));
      return Math.round(number * places) / places;
    };

    /*
     * Replicates functionality of jQuery.addClass() which doesn't work on svgs.
     */
    self.addClass = function($el, className) {
      $el.attr('class', $el.attr('class') + ' ' + className);
    };

    /*
     * Perform post chart creation tasks.
     */
    self.postBuildCallback = function () {
      // Custom classes that get applied to the svg based on setting.
      var classes = [
        'tc-area-opacity-' + self.settings.areaOpacity,
        'tc-tick-visibility-' + self.settings.tickVisibility
      ];
      // $.addClass does't work here so classes are added as an attribute.
      $('#' + self.settings.chartDomId + ' > svg').attr('class', classes.join(' '));
      // Add styles to legend.
      if (self.settings.styles.length) {
        $(self.settings.styles).each(function (i, d){
          var $legendItem = $('#' + self.settings.chartDomId + ' .c3-legend-item-' + d.set.replace(' ', '-'));
          self.addClass($legendItem, 'c3-style-' + d.style);
        });
      }
      // Execute any callbacks passed from tableCharts.
      self.settings.chartInitCallback();
    };

    // Create the chart.
    self.init();

  };

})(jQuery);
