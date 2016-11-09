<?php
$plugin = array(
  //'single' => TRUE,  // Just do this one, it is needed.
  'title' => t('Academic Program Course Listing'),  // Title to show up on the pane screen.
  'description' => t('Displays course listings for an academic program.'), // Description to show up on the pane screen.
  'category' => t('Swarthmore Site Panes'), // A category to put this under.
  'edit form' => 'acalog_pane_edit_form', // A function that will return the settings form for the pane.
  'render callback' => 'acalog_pane_render', // A function that will return the renderable content.
  'admin info' => 'acalog_pane_admin_info', // A function that will return the information displayed on the admin screen (optional).
  'defaults' => array( // Array of defaults for the settings form.
    'override_title_text' => 'Course Listing',
    'acalog_program' => 0,
    'acalog_num_semesters' => 4,
    'acalog_additional_fields' => array(),
    'acalog_pid' => 0,
    'custom_header' => array(),
    'custom_footer' => array(),
    'component_hide_title' => 0
  ),
  'all contexts' => TRUE, // This is NEEDED to be able to use substitution strings in your pane.
  'edit text' => "Edit",
  'edit form' => 'acalog_pane_edit_form',
);

/**
 * An edit form for the pane's settings.
 */
function acalog_pane_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];

  $form['override_title_text']['#access'] = TRUE;
  $form['override_title']['#access'] = FALSE;
  $form['override_title']['#default_value'] = TRUE;
  $form['override_title_markup']['#access'] = FALSE;
  $form['override_title_heading']['#access'] = FALSE;

  $form['override_title_text']['#title'] = t('Title');
  $form['override_title_text']['#required'] = TRUE;

  $form['component_hide_title'] = array(
    '#type' => "checkbox",
    '#title' => t("Hide title"),
    '#default_value' => (isset($conf['component_hide_title'])) ? $conf['component_hide_title'] : 0,
  );

  $form['acalog_pid'] = array(
    '#type' => "value",
    '#value' => (isset($conf['acalog_pid'])) ? $conf['acalog_pid'] : 0
  );
  $active_catalog = variable_get('acalog_active_catalog', 0);
  $acalog_catalogs = variable_get('acalog_catalogs', array());
  $acalog_programs = variable_get('acalog_programs', array());
  $acalog_course_prefixes = _acalog_pane_get_course_prefixes();
  $acalog_course_prefix_options = array();
  foreach ($acalog_course_prefixes AS $prefix){
    $acalog_course_prefix_options['prefix:' . $prefix] = $prefix;
  }
  if (!empty($acalog_programs[$active_catalog])){
    if (!empty($conf['acalog_catalog_program']) AND !isset($acalog_programs[$active_catalog][$conf['acalog_program']])){
      drupal_set_message(t('Could not find Acalog program %id in current catalog, please select a valid program from catalog %catalog_name.', array('%id' => $conf['acalog_program'], '%catalog_name' => $acalog_catalogs[$active_catalog])), 'error');
    }
    $options = array(0 => "- none set -") + $acalog_programs[$active_catalog];
    $options['Course Prefixes'] = $acalog_course_prefix_options;
    $form['acalog_program'] = array(
        '#type' => "select",
        '#title' => "Academic Program or Course Prefix",
        '#default_value' => (isset($conf['acalog_program'])) ? $conf['acalog_program'] : 0,
        '#options' => $options,
        '#description' => t('Please select an academic program to display.')
    );

    $unique_semesters = acalog_query_unique_catalog_semsters();
   
    $semester_options['all'] = t("Show all courses (not listed by semester)");
    for ($i = 1; $i <= 6; $i++){
      $semester_options[$i] = $i . ' ' . format_plural( $i, 'semester', 'semesters');
    }

    $form['acalog_num_semesters'] = array(
      '#type' => "select",
      '#title' => "Number of semesters to show",
      '#default_value' => (isset($conf['acalog_num_semesters'])) ? $conf['acalog_num_semesters'] : 4,
      '#options' => $semester_options,
      '#description' => t('Note: only current and future semesters will be displayed starting with the %current semester.', array('%current' => ((date('m') < 6)? 'Spring ' : 'Fall ') . date('Y')))
    );

    $templates = variable_get('acalog_course_template', array());
    $visible_fields = $templates[$active_catalog];
    $field_options = array();
    foreach ($visible_fields AS $field_name=>$opts){
      if (!in_array(strtolower(rtrim($field_name, '.')), $unique_semesters) AND !in_array($field_name, array('Catalog chapter:', 'Department website:'))){
        $field_options[$field_name] = $field_name;
      }
    }
    $form['acalog_additional_fields'] = array(
      '#type' => "checkboxes",
      '#title' => "Additional course fields to display",
      '#default_value' => (isset($conf['acalog_additional_fields'])) ? $conf['acalog_additional_fields'] : array(),
      '#options' => $field_options
    );
  } else {
    $form['programs_na'] = array(
      '#markup' => t("No academic programs could be found.")
    );
  }

  $form['custom_header_footer'] = array(
    '#title' => t('Custom header and footer HTML'),
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#id' => "acalog-course-custom-header-footer",
    '#prefix' => '<style type="text/css">#acalog-course-custom-header-footer .filter-wrapper {display: none;}</style>'
  );
  $form['custom_header_footer']['custom_header'] = array(
    '#type' => 'text_format',
    '#title' => t('Custom Header'),
    '#default_value' => (isset($conf['custom_header']['value'])) ? $conf['custom_header']['value'] : "",
  );
  $form['custom_header_footer']['custom_footer'] = array(
    '#type' => 'text_format',
    '#title' => t('Custom Footer'),
    '#default_value' => (isset($conf['custom_footer']['value'])) ? $conf['custom_footer']['value'] : "",
  );
  return $form;
}

/**
 * Submit function, note anything in the formstate[conf] automatically gets saved
 * Notice, the magic that automatically does that for you.
 */
function acalog_pane_edit_form_submit(&$form, &$form_state) {
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    if (isset($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
} 

/**
 * Run-time rendering of the body of the block (content type)
 * See ctools_plugin_examples for more advanced info
 */
function acalog_pane_render($subtype, $conf, $args, $contexts) {
  $pane = new stdClass();  
  
  //$pane->title = (empty($conf['component_hide_title'])) ? $conf['override_title_text'] : '<none>';
  $pane->title = '<none>';

  $pane->content = theme("acalog_course_listing", $conf);
  
  // Add contextual links for the webform node 
  $pane->content['#contextual_links'] = array(
    'courses-panel-pane' => array('courses-panel-pane', array($conf['acalog_pid']))
  );
    
  return $pane;
}

/**
 * 'admin info' callback for panel pane.
 */
function acalog_pane_admin_info($subtype, $conf, $contexts) {
  
  $active_catalog = variable_get('acalog_active_catalog', 0);
  $acalog_programs = variable_get('acalog_programs', array());
  $program = (isset($acalog_programs[$active_catalog][$conf['acalog_program']])) ? $acalog_programs[$active_catalog][$conf['acalog_program']] : t('No Program Selected');
  $pane = new stdClass();
 
  $pane->title = $conf['override_title_text'];
  $pane->content = theme('item_list', array('items' => array(
      t('Program: %program', array('%program' => $program)),
      t('Displaying %semesters semesters', array('%semesters' => $conf['acalog_num_semesters']))
    )));

  return $pane;
}



