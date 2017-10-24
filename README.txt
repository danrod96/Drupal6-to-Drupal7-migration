Drupal6 to Drupal 7 Integration
-------------------------------


This is a set of Drupal 6 and Drupal 7 modules that we created to integrate two websites, http://clasificados.eltiempo.com (based in Drupal 6) and https://seg.eltiempo.com (Drupal 7).

Basically, for each website, there is a webservice which logs users, registers users, and logs off users. So if I log in Drupal 6, there is a call to the webservice on Drupal 7 for logging in Drupal 7 as well. 

The same situation if I log in Drupal 7 first, from this website we use the webservice from Drupal 6 for logging in Drupal 6 too.