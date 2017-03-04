# Drupal Acalog Module
Drupal 7.x module that uses the Acalog Course Catalog API to pull course information into a Drupal database.

## Installation
1 Clone this repository in your Drupal modules directory
2 Enable this module
3 Visit admin/config/system/acalog/api and provide:
  - Your Acalog catalog URL - the URL for your public facing catalog
  - Your Acalog API URL - API URLs are in the form http://{acalogclientname}.apis.aclog.com
  - A public Acalog API key - Note: private keys should not be used in production environments
4 Visit admin/config/system/acalog, the "Active Acalog Catalog" selection field will be populated with your published catalogs if the Acalog API was able to connect
5 Click "Acalog Catalog Data Refresh" to your Drupal database with the course and program data from the catalog selected in step 4

## Displaying course info
This module provides a Swarthmore College specific panel pane titled "Academic Program Course Listing." The pane is intended to list a programs courses grouped by the semester they are being offered.

To adapt this module for your institution, it would be recommended to either wrap the provided panel pane output in a block (for use on block based Drupal sites,) or use hook_view_data to provide program and course data to the Drupal Views module API.

This module was presented at DigArcU 2017 - presentation slide can be found @ https://docs.google.com/presentation/d/1Hczvp9hKG6G9zL53pJs_-OglLRVGahPwE5ooIY__k3Y
