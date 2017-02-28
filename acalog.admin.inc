<?php
/**
 * The acalog catalog module setting form
 */
function acalog_settings_form(){
  $catalogs = variable_get('acalog_catalogs', array());

  if (empty($catalogs)){
    $catalogs = acalog_fetch_all_catalogs();
  }

  $form = array();

  $active_catalog = variable_get('acalog_active_catalog', 0);
  if ($active_catalog){
    $form['acalog_data_refresh'] = array(
      '#type' => "fieldset",
      '#description' => t('Refresh all updated programs and courses in the %cat_name catalog.', array('%cat_name' => $catalogs[$active_catalog]))
    );
    $form['acalog_data_refresh']['acalog_data_batch_refresh'] = array(
        '#type' => 'submit',
        '#value' => t('Acalog Catalog Data Refresh'),
        '#submit' => array('acalog_data_batch_refresh_submit'),
    );
    $acalog_programs = variable_get('acalog_programs', array());
    if (!empty($acalog_programs[$active_catalog])){


      $form['acalog_program_data_refresh'] = array(
        '#type' => "fieldset",
        '#description' => t('Refresh all programs and course data for a selected program in the %cat_name catalog.', array('%cat_name' => $catalogs[$active_catalog]))
      );
      $options = array(0 => "- none selected -") + $acalog_programs[$active_catalog];
      $form['acalog_program_data_refresh']['acalog_program_data_refresh_select'] = array(
        '#type' => "select",
        '#title' => "Academic Program",
        '#default_value' => (isset($conf['acalog_program'])) ? $conf['acalog_program'] : 0,
        '#options' => $options,
        '#description' => t('Please select an academic program to update.')
      );
      $form['acalog_program_data_refresh']['acalog_program_data_batch_refresh'] = array(
          '#type' => 'submit',
          '#value' => t('Acalog Program Data Refresh'),
          '#submit' => array('acalog_program_data_batch_refresh_submit'),
      );
    }
  }

  $form['acalog_last_active_catalog'] = array(
    '#type' => "value",
    '#value' => $active_catalog
  );

  $form['acalog_active_catalog'] = array(
    '#type' => "select",
    '#title' => t("Active Acalog Catalog"),
    '#default_value' => $active_catalog,
    '#options' => $catalogs,
    '#description' => t('Set the current Acalog catalog to pull course information')
  );

  $form['#submit'][] = 'acalog_settings_form_catalog_change_submit';

  return system_settings_form($form);
}

/**
 * See if we need to update acalog catalog data and data panes after a setting change.
 */
function acalog_settings_form_catalog_change_submit($form, $form_state){
  if ($form_state['values']['acalog_last_active_catalog'] != $form_state['values']['acalog_active_catalog']){
    //make sure the active catalog is set before doing the batch process update
    variable_set('acalog_active_catalog', $form_state['values']['acalog_active_catalog']);
    //update the course data.
    acalog_data_batch_refresh_submit($form, $form_state);
    //now update the course panes.
    $active_catalog = $form_state['values']['acalog_active_catalog'];
    $last_active_catalog = $form_state['values']['acalog_last_active_catalog'];
    $acalog_programs = variable_get('acalog_programs', array());
    $query = db_select('panels_pane', 'pp')->fields('pp', array('pid', 'configuration'))->condition('type', "acalog_pane", '=');
    $results = $query->execute();
    foreach ($results AS $pane){
      $pane->configuration = unserialize($pane->configuration);
      if (!empty($pane->configuration['acalog_program'])){
        //if the program_id is in the current catalog then there is nothing to do
        if (isset($acalog_programs[$active_catalog][$pane->configuration['acalog_program']])){
          continue;
        }
        $new_program_id = array_search($acalog_programs[$last_active_catalog][$pane->configuration['acalog_program']], $acalog_programs[$active_catalog]);
        if (!$new_program_id){ //lets try and retrieve from the DB, may be in another catalog.
          $query = db_select('acalog_catalog_program', 'p')->fields('p', array('catalog_id'))->condition('program_id', $pane->configuration['acalog_program'], "=");
          if ($catalog_id = $query->execute()->fetchField()){
            $new_program_id = array_search($acalog_programs[$catalog_id][$pane->configuration['acalog_program']], $acalog_programs[$active_catalog]);
          }
        }
        if ($new_program_id){
          //save the pane
          $pane->configuration['acalog_program'] = $new_program_id;
          drupal_write_record('panels_pane', $pane, array('pid'));
        } else {
          //log this
          watchdog("acalog", "Unable to find program with ID of @program_id set in panels pane ID @pane_id when changing to catalog ID @catalog_id", array('@program_id' => $pane->configuration['acalog_program'],'@pane_id' => $pane->pid, '@catalog_id' => $active_catalog), WATCHDOG_NOTICE);
        }
      }
    }
  }
}

/**
 * Batch process all acalog catalog data
 */
function acalog_data_batch_refresh_submit($form, $form_state){
  $batch = array(
    'operations' => _acalog_batch_cron_operations(),
    'finished' => 'acalog_data_batch_refresh_finished',
    'title' => t('Updating all catalog data.'),
    'init_message' => t('Catalog update is starting.'),
    'progress_message' => t('Completed processing @percentage% of catalog data.'),
    'error_message' => t('Catalog update has encountered an error.'),
  );

  batch_set($batch);
}

/**
 * Batch process all acalog program data
 */
function acalog_program_data_batch_refresh_submit($form, $form_state){
  if (!empty($form_state['values']['acalog_program_data_refresh_select'])){
    $batch = array(
      'operations' => array(
        array("acalog_single_program_data_update", array($form_state['values']['acalog_program_data_refresh_select'])),
      ),
      'finished' => 'acalog_data_batch_refresh_finished',
      'title' => t('Updating program data.'),
      'init_message' => t('Program update is starting.'),
      'progress_message' => t('Completed processing @percentage% of program data.'),
      'error_message' => t('Program update has encountered an error.'),
    );

    for ($i = 0; $i < 5; $i++){
      $batch['operations'][] = array('acalog_course_data_update', array($i));
    }

    batch_set($batch);
  }
}

/**
 * The acalog API admin settings form
 */
function acalog_api_settings_form(){
  $form = array();

  $form['acalog_api_url'] = array(
      '#type' => 'textfield',
      '#title' => t('Acalog API URL'),
      '#default_value' => variable_get('acalog_api_url', "http://XXXXXXXX.apis.acalog.com"),
      '#description' => t("API URLs are in the form http://{acalogclientname}.apis.aclog.com")
  );

  $form['acalog_api_key'] = array(
      '#type' => 'textfield',
      '#title' => t('Acalog API key'),
      '#default_value' => variable_get('acalog_api_key', ""),
      '#description' => t("For production sites only use the public acalog key")
  );

  return system_settings_form($form);
}
