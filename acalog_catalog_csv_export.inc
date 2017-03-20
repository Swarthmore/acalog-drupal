<?php

function acalog_course_csv_export(){
  $query = db_select("acalog_catalog_course_meta", "m")->fields('m', array("meta_name"))->distinct();
  $result = $query->execute();
  $header = $result->fetchCol();
  asort($header);
  $header = array_values($header);
  $courses = array();
  $courses[0] = $header;
  $query = db_select("acalog_catalog_course_meta", "m")->fields('m', array("course_id","meta_name","meta_data"));
  $result = $query->execute();
  while($row = $result->fetchAssoc()){
    $id = $row['course_id'];
    if (!isset($courses[$id])){
      $courses[$id] = array();
    }
    $index = array_search($row['meta_name'], $header);
    $courses[$id][$index] = $row['meta_data'];
  }
  //fill in the blanks
  foreach ($courses AS $cid=>$course){
    for ($index = 0; $index < count($header); $index++){
      if (!isset($course[$index])){
        $courses[$cid][$index] = "";
      }
    }
    //sort by keys
    ksort($courses[$cid], SORT_NUMERIC);
  }

  if ($temp = drupal_tempnam('temporary://', 'acalogdl_')){
		$tcsv = fopen($temp, "w");
    foreach ($courses AS $row){
      fputcsv($tcsv, $row);
    }
  }
  file_transfer($temp, array('Content-disposition' => 'attachment; filename=acalog_courses.csv'));
  exit;
}
