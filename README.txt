Drupal6 to Drupal 7 Integration
-------------------------------


This is a set of Drupal 6 and Drupal 7 modules that we created to integrate two websites, http://clasificados.eltiempo.com (Drupal 6 website) and https://seg.eltiempo.com (Drupal 7 website).

The original idea was to enable SSO integration between both sites, so the user only needed to log in once for getting access to the other website.

Basically, for each website, there is a webservice which logs in users, registers users, and logs off users, therefore if I log in the Drupal 6 website, there is a call to the webservice on Drupal 7 for logging in Drupal 7 as well, and vice versa. 