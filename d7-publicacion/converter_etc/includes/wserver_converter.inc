<?php
/**
 * @file
 * 
 * Webservice methods
 *
 * Version 1.0 - 16/10/2014
 * - Initial version
 * 
 * Version 2.0 - 14/01/2015
 * - extra bugfixes requested
 * - New method actualizarUsuario
 * 
 * Version 2.3 - 23/01/2015
 * - new field: TOC
 * 
 */

 
  /**
  * Function existeUsuario
  * 
  * Funcion que valida la existencia de usuario dentro de Drupal 
  * Checks if the username exists or not
  */
 function existeUsuario($user)
 {
 	$response = array("codigo" => "0","msg" => "No existe usuario");
 	
 	$usuario = user_load_by_name($user);
 	
 	if ($usuario){
 		$response = array("codigo" => "1","msg" => "existe usuario");
 	} 
 	
 	return json_encode($response);
 }
 
 /**
  * Function crearUsuario
  * 
  * Creates Drupal user
  *
  * Input: mail, username, password, first name, last name, phone number, id type, id number, UID of Drupal 6
  */
 function crearUsuario($correo, $usuario, $contrasena, $nombre, $apellido, $telefono, $tipodoc, $numdoc, $uid6)
 {
  	require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');

 	$response = array();
 	
	$account = new stdClass();
	$account->is_new = true;
	
  	//convert Drupal 6 password format to Drupal 7 password format
  	$pass_d7 ='U'.user_hash_password($contrasena, 11);

  	$account = new StdClass();
  	$account->is_new = TRUE;
  	$account->status = TRUE;
  	$account->name = $usuario;
  	$account->pass = $pass_d7;
  	$account->mail = $correo;
  	$account->init = $correo;
  	$account->roles = array(DRUPAL_AUTHENTICATED_RID => TRUE);
  	$account->field_user_name[LANGUAGE_NONE][0]['value'] = $nombre;
  	$account->field_user_last[LANGUAGE_NONE][0]['value'] = $apellido;
  	$account->field_phone_number[LANGUAGE_NONE][0]['value'] = $telefono;
  	$account->field_id_type[LANGUAGE_NONE][0]['tid'] = intval($tipodoc);
  	$account->field_id_number[LANGUAGE_NONE][0]['value'] = $numdoc;
  	$account->field_uid_d6[LANGUAGE_NONE][0]['value'] = $uid6;
  	$account->field_tocs[LANGUAGE_NONE][0]['value'] = 1;
    
    try {
      $new_user = user_save($account);
      $response = array("codigo" => "1","msg" => "Creado usuario con uid drupal 6: " . $uid6);
    } catch (Exception $e) {
      $response = array("codigo" => "0","msg" => "No se creo el usuario");
      watchdog("Converter", "El usuario " . $usuario . " no fue creado", WATCHDOG_NOTICE);
    }
    
    return json_encode($response);
 }
 
  /**
  * Function actualizarUsuario
  * 
  * Updates user account data
  * 
  * Input: first name, last name, phone number, id type, id number, Drupal 6 UID
  */
 function actualizarUsuario($nombre, $apellido, $telefono, $tipodoc, $numdoc, $uid6)
 {
 	global $user;
 	
 	$language = "LANGUAGE_NONE";
 	
 	$response = array("codigo" => "0", "msg" => "Error al actualizar datos de usuario");
 	
 	//find drupal 7 uid associated with the Drupal 6 UID
 	$tmp = db_query('SELECT entity_id FROM {field_data_field_uid_d6}  WHERE field_uid_d6_value = :numid', array(':numid' => $uid6));
 	
 	$uid = $tmp->fetchField();
 	
 	if (!$uid){
 		$response = array("codigo" => "-1", "msg" => "El UID de Drupal 6 no existe en Drupal 7");
 		return json_encode($response);
 	}
 	
 	$account = user_load($uid);
 	
 	//new data
 	$datos = array(
		  'field_user_name' => array(
	            $language => array(
	              	0 => array(
	                  'value' => $nombre,
	              	),
	            ),
	        ),
	    	'field_user_last' => array(
	            $language => array(
	              	0 => array(
	                  'value' => $apellido,
	              	),
	            ),
	        ),
	    	'field_phone_number' => array(
	            $language => array(
	              	0 => array(
	                  'value' => $telefono,
	              	),
	            ),
	        ),
	        //Seleccionar el termino de la taxonomia correcta
	    	'field_id_type' => array(
	            $language => array(
	              	0 => array(
	                  'tid' => intval($tipodoc),
	              	),
	            ),
	        ),
	    	'field_id_number' => array(
	            $language => array(
	              	0 => array(
	                  'value' => $numdoc,
	              	),
	            ),
	        ),
 	);
 	
 	$res = user_save($account, $datos);
 	
 	if (!$res){
 		return json_encode($response);
 	} else {
 		$response = array("codigo" => "1", "msg" => "Se ha actualizado el usuario con el uid local: " . $uid . ", uid drupal 6: " . $uid6 . " y numero de documento " . $numdoc);
 		return json_encode($response);	
 	}
 	
 }

/**
 * Function logout7
 * 
 * Logs out user from Drupal 7
 *
 * Input: Drupal 6 UID
 */
function logOut7($uid6)
{ 	  
	$client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

 	$tmp = db_query('SELECT entity_id FROM {field_data_field_uid_d6}  WHERE field_uid_d6_value = :numid', array(':numid' => $uid6));
 	
 	$uid = $tmp->fetchField();
 	
 	if (!$uid){
 		$response = array("codigo" => "-1", "msg" => "El UID de Drupal 6 no existe en Drupal 7");
 		return json_encode($response);
 	}
 	
	//finding the SID
 	$tmp2 = db_query('SELECT sid FROM {sessions}  WHERE uid = :numid ORDER BY timestamp DESC', array(':numid' => $uid));

	$sid7 = $tmp2->fetchField();
	
	if (!$sid7){
 		$response = array("codigo" => "-2", "msg" => "Usuario no autenticado en Drupal 7");
 		return json_encode($response);
 	} 
	
	//delete SID row
	$sess_borrado = db_delete('sessions')
	 ->condition('uid', $uid)
	 ->execute();
	 
	 if (!$sess_borrado){
	 	$response = array("codigo" => "0", "msg" => "No se deslogueo de Drupal 7");
	 	return json_encode($response);
	 } else {
	 	$response = array("codigo" => "1", "msg" => "Se deslogueo de Drupal 7");
	 	return json_encode($response);
	 } 
}

  /**
  * Function logIn7
  *
  * Funcion que realiza el proceso de logueo.
  * Logs In user in Drupal 7
  * Argumentos de entrada:
  *
  * Input: user email and password.
  */
   function logIn7($correo_ext, $contrasena)
  {
  	  $response = array("codigo" => "-1", "msg" => "No hay nombre de usuario asociado al correo o no existe usuario");

  	  global $cookie_domain;
  	  global $user;

  	  //find username and uid from email
  	  //username = email
	  $query_obtener_usuario = db_query('SELECT name, uid FROM {users}  WHERE name = :username', array(':username' => $correo_ext));

	  $info = $query_obtener_usuario->fetchAll();

	  foreach ($info as $tmp){
	  	$uid = $tmp->uid;
	  	$name = $tmp->name;
	  }

	  //if username != email, find by mail
	  if (is_null($name)){
	  	  $query_obtener_usuario2 = db_query('SELECT name, uid FROM {users} WHERE mail = :mailuser', array(':mailuser' => $correo_ext));
	  	  $info = $query_obtener_usuario2->fetchAll();

	  	  foreach ($info as $tmp){
	  	    $uid = $tmp->uid;
	  	    $name = $tmp->name;
	      }

		  if (is_null($name)){
		  	return json_encode($response);
		  } 
	  } 

	  //call to user_authenticate
	  $info_usuario = user_authenticate($name, $contrasena);

	  if(!$info_usuario){
	  	$response = array("codigo" => "0", "msg" => "Credenciales incorrectas");
	  	return json_encode($response);
	  } else {
      	$user = user_load($info_usuario);

      	$login_array = array ('name' => $name, 'pass' => $contrasena);

      	//finalize login
      	user_login_finalize($login_array);

	  	$response = array("codigo" => "1", "uid" => $user->uid, "timestamp" => $user->login, "msg" => "Usuario logueado");

	  	return json_encode($response);
	  } 
  }

 /**
  * Function getSessionNme
  *
  * Returns Session ID
  */
 function getSessionName()
 {
   	return session_name();
 }

 /**
  * Function isLogged
  *
  * Check if the user is logged into the Drupal 7 website
  *
  * INPUT: SID
  */
 function isLogged($cookie_session_value)
 {
 	$response = array("codigo" => "0","msg" => "No esta logueado");

 	if (empty($cookie_session_value)){
 		return json_encode($response);
 	}

 	//query para obtener el registro asociado al sid del argumento
 	//get uid from SID
    $query_obtener_sid = db_query('SELECT uid, sid FROM {sessions} WHERE sid = :siduser', array(':siduser' => $cookie_session_value));
 	$info = $query_obtener_sid->fetchAll();

	 foreach ($info as $tmp){
	  	$uid = $tmp->uid;
	  	$sid = $tmp->sid;
	 }

 	if ($uid == 0){
 		return json_encode($response);
 	} else if ($uid > 0 && !is_null($sid)){
 		//user is logged in
 		$response = array("codigo" => "1", "msg" => "Esta logueado");
 		return json_encode($response);
 	} else if (is_null($uid)){
 		//no session for now
 		return json_encode($response);
 	} else {
 		//error
 		return json_encode($response);
 	}

 }

 /**
  * Function buildCookie
  *
  * Returns SID
  * 
  * INPUT: uid and login timestamp
  */
 function buildCookie($uid, $login)
 {
	$query_obtener_sid = db_query('SELECT sid FROM {sessions} WHERE uid = :uiduser AND timestamp = :tstamp', array(':uiduser' => $uid, ':tstamp' => $login));

	$sid7 = $query_obtener_sid->fetchField();

	if(is_null($sid7)) {
	  $response = array("codigo" => "0", "msg" => "Sesion no iniciada");
	} else {
	  $response = array("codigo" => $sid7, "msg" => "Sesion iniciada");
	}
	return json_encode($response);
 }
