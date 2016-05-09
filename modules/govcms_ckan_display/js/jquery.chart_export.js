(function ($) {

  "use strict";

  /*
   * ChartExport jQuery plugin.
   * ---------------------------
   * This plugin handles exporting a chart to a svg/png format.
   *
   * Usage.
   * ------
   * Bind this to the button you wish to trigger the download of the chart.
   * Eg. $('button).chartExport({svg: 'svg.my-chart', format: 'svg'});
   *
   * Options/settings:
   * - svg: (required) This is either a jQuery object or jQuery selector for the chart.
   * - format: (string) The save as format (svg or png). Defaults to svg
   * - filename: (string) The filename to use when saving, Defaults to 'chart'
   * - includeC3jsStyles: (bool) Should we inject some c3js styles that fix output bugs. Defaults to true
   * - errorMsg: (string) The error message to display if no browser support for downloading.
   */

  /**
   * chartExport class.
   *
   * @param dom
   *   The dom for the triggering element (eg. the button/link).
   * @param settings
   *   Settings for the class. @see defaults.
   * @returns {chartExport}
   *   Return self for chaining.
   */
  var ChartExport = function (dom, settings) {
    var self = this;

    // Defaults.
    self.defaults = {
      // The button this is getting bound to.
      $button: $(dom),
      // Chart SVG selector or jQuery object.
      svg: 'svg',
      // Format to save as.
      format: 'svg',
      // Filename (without extension).
      filename: 'chart',
      // Export as png dimensions. When values are empty, will automatically size.
      width: '',
      height: '',
      // Include c3js styles (fixes display bugs with c3js charts)
      includeC3jsStyles: true,
      c3jsStyles: 'svg{font:10px sans-serif}line,path{fill:none;stroke:#000}.c3-bar{stroke:none!important}',
      // Passed Validation requirements.
      valid: true,
      // Error message.
      errorMsg: 'Sorry, your browser does not support this function.',
      // Manual download message.
      manualDownloadMessage: 'Your browser requires manual saving of this file, when redirected to the image, ' +
        'right click and "Save {saveType} As..." to save it to your computer with the filename "{filename}"'
    };

    // Update defaults with passed settings.
    self.settings = $.extend(self.defaults, settings);

    /*
     * Validate requirements.
     */
    self.validateRequirements = function () {
      // TODO: Check html5 support.
      try {
        var isFileSaverSupported = !!new Blob;
      } catch (e) {
        self.settings.valid = false;
      }

      // Return self for chaining.
      return self;
    };

    /*
     * Parse the svg object.
     */
    self.parseSvg = function () {
      // Processed class.
      var processedClass = 'chart-export-processed';

      // If svg is not a jQuery object, make it one.
      if (!self.settings.svg instanceof jQuery) {
        self.settings.svg = $(self.settings.svg);
      }

      // Ensure our svg is actually an svg (not a wrapper element).
      self.settings.svg = self.settings.svg.is('svg') ? self.settings.svg : self.settings.svg.find('> svg');

      // If no svg, validation failed.
      if (self.settings.svg.length === 0 || self.settings.svg.hasClass(processedClass)) {
        self.settings.valid = false;
        return;
      }

      // Add some XML attributes.
      self.settings.svg
        .attr('version', 1.1)
        .attr('xmlns', 'http://www.w3.org/2000/svg')
        .find('g').removeAttr('clip-path')
        .find('text').attr('font-family', '\'arial\'');

      // Include c3js styles.
      self.includeC3jsStyle();

      // Prevent duplicate parsing.
      self.settings.svg.addClass(processedClass);

      // Return self for chaining.
      return self;
    };

    /*
     * Inject c3js styles if required.
     */
    self.includeC3jsStyle = function () {
      // Check if styles need to be added first.
      if (!self.settings.includeC3jsStyles) {
        return;
      }

      // Create a style element.
      self.$c3styles = $('<style>')
        .attr('type', 'text/css')
        .html("<![CDATA[\n" + self.settings.c3jsStyles + "\n]]>")
        .appendTo($('defs', self.settings.svg));

      // Return self for chaining.
      return self;
    };

    /*
     * Save as SVG.
     */
    self.saveSVG = function () {
      // Save using blob and filesaver.js.
      var blob = new Blob([self.svgHtml], {type: "image/svg+xml"});
      saveAs(blob, self.getFilename());
    };

    /*
     * Save as PNG.
     */
    self.savePNG = function () {
      // Create a new image and add the svg as a src.
      var image = new Image();
      image.src = 'data:image/svg+xml;base64,' + btoa(self.svgHtml);

      // On image load, trigger its download with html5 download attr.
      image.onload = function () {
        // Once loaded, turn the image object into a 2d canvas.
        var canvas = document.createElement('canvas'), context, dimensions;
        dimensions = self.getDimensions(image);
        canvas.width = dimensions.width;
        canvas.height = dimensions.height;
        context = canvas.getContext('2d');
        context.drawImage(image, 0, 0, dimensions.width, dimensions.height);

        // Save using canvas-toBlob.js.
        canvas.toBlob(function (blob) {
          saveAs(blob, self.getFilename());
        });
      };
    };

    /*
     * Returns the width and height for the canvas based on settings and AR.
     *
     * @param image
     *   The image with the src populated as the chart.
     */
    self.getDimensions = function (image) {
      // Default to the image width/height.
      var d = {width: image.width, height: image.height}, ar;

      // If overridden we do some additional processing with the settings.
      if (self.settings.width !== '') {
        // Width set, height optional.
        d.width = parseInt(self.settings.width);

        // If no height, we calculate with the aspect ratio.
        if (self.settings.height === '') {
          ar = (image.height / image.width);
          d.height = Math.round((d.width * ar));
        } else {
          // Defined width and height: May result in cropping or poor AR.
          d.height = parseInt(self.settings.height);
        }

      } else if (self.settings.height !== '') {
        // No width, only height defined, calculate width with AR.
        d.height = parseInt(self.settings.height);
        ar = (image.width / image.height);
        d.width = Math.round((d.height * ar));
      }

      // Return the dimensions object.
      return d;
    };

    /*
     * Click action.
     */
    self.bindClick = function (e) {
      e.preventDefault();

      // If not passed validation, clicking the button returns an error msg.
      if (self.settings.valid === false) {
        return alert(self.settings.errorMsg);
      }

      // Safari currently doesn't support filesaver
      // @see https://github.com/eligrey/FileSaver.js/issues/12
      // So as a workaround we instruct the user on how to download.
      if (self.isSafari()) {
        var saveType = 'png' === self.settings.format ? 'Image' : 'Page';
        alert(self.settings.manualDownloadMessage.replace('{filename}', self.getFilename()).replace('{saveType}', saveType));
      }

      // Get the html for the svg.
      self.svgHtml = self.settings.svg[0].outerHTML;

      // Call the save method based on format.
      switch (self.settings.format.toLowerCase()) {
        case 'svg':
          self.saveSVG();
          break;
        case 'png':
          self.savePNG();
          break;
      }
    };

    /*
     * Safari check due to issue with filesaver.
     */
    self.isSafari = function () {
      return navigator.vendor && navigator.vendor.indexOf('Apple') > -1 && navigator.userAgent && !navigator.userAgent.match('CriOS');
    };

    /*
     * Return the filename with extension.
     */
    self.getFilename = function () {
      return self.settings.filename + '.' + self.settings.format;
    };

    /*
     * Init the class.
     */
    self.init = function () {
      // Only one export per button.
      if (self.settings.$button.hasClass('chart-export-processed')) {
        return;
      }

      // Prepare requirements and svg.
      self
        .validateRequirements()
        .parseSvg();

      // Bind click and mark as processed.
      self.settings.$button
        .click(self.bindClick)
        .addClass('chart-export-processed');
    };

    // Kick it off on constuct.
    self.init();

    // Return self for chaining.
    return self;
  };


  /**
   * jQuery plugin/function.
   */
  $.fn.chartExport = function (settings) {
    return this.each(function (i, dom) {
      new ChartExport(dom, settings)
    });
  };

})(jQuery);
