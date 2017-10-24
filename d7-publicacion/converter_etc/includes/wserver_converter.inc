<?php
/**
 * @file
 * 
 * Funciones que realizan operaciones de usuarios: creacion de usuarios, validacion existencia de usuarios, entre otros
 *
 * Version 1.0 - 16/10/2014
 * - Version inicial
 * 
 * Version 2.0 - 14/01/2015
 * - Ajustes solicitados
 * - Funcion actualizarUsuario
 * 
 * Version 2.3 - 23/01/2015
 * - Registrar valor TOC
 * 
 */

 
  /**
  * funcion existeUsuario
  * 
  * Funcion que valida la existencia de usuario dentro de Drupal 
  * 
  * Devuelve:
  *  json 0 - Si no existe usuario
  *  json 1 - Si existe usuario
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
  * funcion crearUsuario
  * 
  * Funcion que crea un usuario de acuerdo con los valores de entrada dados
  * 
  * Devuelve:
  *  json 0 - Usuario no fue creado
  *  json 1 - Usuario creado
  */
 function crearUsuario($correo, $usuario, $contrasena, $nombre, $apellido, $telefono, $tipodoc, $numdoc, $uid6)
 {
  require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');

 	$response = array();
 	
	$account = new stdClass();
	$account->is_new = true;
	
	//el lenguaje que seria por defecto en el drupal
	//debe establecerse a 'es' si el sitio esta en espanol
	$language = 'und';

  //Conversion de pass drupal 6 a formato drupal 7
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
      //registro en watchdog
      watchdog("Converter", "El usuario " . $usuario . " no fue creado", WATCHDOG_NOTICE);
    }
    
    return json_encode($response);
 }
 
  /**
  * funcion actualizarUsuario
  * 
  * Funcion que actualiza un usuario de acuerdo con los valores de entrada dados
  * 
  * Devuelve:
  *  json 0 - Usuario no fue actualizado
  *  json 1 - Usuario actualizado
  */
 function actualizarUsuario($nombre, $apellido, $telefono, $tipodoc, $numdoc, $uid6)
 {
 	global $user;
 	
 	$language = "und";
 	
 	$response = array("codigo" => "0", "msg" => "Error al actualizar datos de usuario");
 	
 	//buscar el usuario asociado al UID de Drupal 6
 	$tmp = db_query('SELECT entity_id FROM {field_data_field_uid_d6}  WHERE field_uid_d6_value = :numid', array(':numid' => $uid6));
 	
 	$uid = $tmp->fetchField();
 	
 	if (!$uid){
 		$response = array("codigo" => "-1", "msg" => "El UID de Drupal 6 no existe en Drupal 7");
 		return json_encode($response);
 	}
 	
 	//hacer la actualizacion
 	$account = user_load($uid);
 	
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
 * funcion logout
 * 
 * funcion que desloguea el usuario 
 */
function logOut7($uid6)
{ 	  
	$client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

	//obtener el uid7 relacionado al uid6
 	$tmp = db_query('SELECT entity_id FROM {field_data_field_uid_d6}  WHERE field_uid_d6_value = :numid', array(':numid' => $uid6));
 	
 	$uid = $tmp->fetchField();
 	
 	if (!$uid){
 		$response = array("codigo" => "-1", "msg" => "El UID de Drupal 6 no existe en Drupal 7");
 		return json_encode($response);
 	}
 	
	//con el valor de uid de drupal 7, buscar en sessions el sid de drupal 7 correspondiente y que sea reciente
 	$tmp2 = db_query('SELECT sid FROM {sessions}  WHERE uid = :numid ORDER BY timestamp DESC', array(':numid' => $uid));

	$sid7 = $tmp2->fetchField();
	
	if (!$sid7){
 		$response = array("codigo" => "-2", "msg" => "Usuario no autenticado en Drupal 7");
 		return json_encode($response);
 	} 
	
	//del sid ir a la tabla sessions y borrar el registro
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
  * funcion logIn7
  *
  * Funcion que realiza el proceso de logueo.
  * Argumentos de entrada:
  * correo_ext: correo electronico
  * contrasena: contrasena del usuario
  *
  * Devuelve:
  * json -1: Si el name es vacio, osea si no hay nombre de usuario asociado al correo
  * json 0: Si no se logueo correctamente
  * json Entero > 1: Si se logueo correctamente
  */
   function logIn7($correo_ext, $contrasena)
  {
  	  $response = array("codigo" => "-1", "msg" => "No hay nombre de usuario asociado al correo o no existe usuario");

  	  global $cookie_domain;
  	  global $user;

  	  //buscar por nombre de usuario
	  $query_obtener_usuario = db_query('SELECT name, uid FROM {users}  WHERE name = :username', array(':username' => $correo_ext));

	  $info = $query_obtener_usuario->fetchAll();

	  foreach ($info as $tmp){
	  	$uid = $tmp->uid;
	  	$name = $tmp->name;
	  }

	  //buscar por correo si por nombre no encontro nada
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

	  //realizar proceso autenticacion
	  $info_usuario = user_authenticate($name, $contrasena);

	  if(!$info_usuario){
	  	//no se logueo correctamente
	  	$response = array("codigo" => "0", "msg" => "Credenciales incorrectas");
	  	return json_encode($response);
	  } else {
	  	global $user;
      	$user = user_load($info_usuario);

      	$login_array = array ('name' => $name, 'pass' => $contrasena);
      	user_login_finalize($login_array);

	  	$response = array("codigo" => "1", "uid" => $user->uid, "timestamp" => $user->login, "msg" => "Usuario logueado");
	  	return json_encode($response);
	  } 
  }

    /**
  * funcion getSessionNme
  *
  * Funcion que devuelve el ID de sesion
  */
 function getSessionName()
 {
   	return session_name();
 }

 /**
  * funcion isLogged
  *
  * Funcion que valida la cookie D7 que recibe como argumento y determina si el usuario esta logueado o no dentro de drupal
  *
  * Devuelve:
  *  0 - Si no esta logueado
  *  1 - Si esta logueado
  */
 function isLogged($cookie_session_value)
 {
 	$response = array("codigo" => "0","msg" => "No esta logueado");

 	if (empty($cookie_session_value)){
 		//argumento cadena vacia
 		return json_encode($response);
 	}

 	//query para obtener el registro asociado al sid del argumento
    $query_obtener_sid = db_query('SELECT uid, sid FROM {sessions} WHERE sid = :siduser', array(':siduser' => $cookie_session_value));
 	$info = $query_obtener_sid->fetchAll();

	 foreach ($info as $tmp){
	  	$uid = $tmp->uid;
	  	$sid = $tmp->sid;
	 }

 	if ($uid == 0){
 		//usuario anonimo cuando uid es 0
 		return json_encode($response);
 	} else if ($uid > 0 && !is_null($sid)){
 		//esta logueado
 		$response = array("codigo" => "1", "msg" => "Esta logueado");
 		return json_encode($response);
 	} else if (is_null($uid)){
 		//no existe registro en la tabla sessions, todavia no tiene sesion, ni como usuario anonimo
 		return json_encode($response);
 	} else {
 		//alguna condicion no validada
 		return json_encode($response);
 	}

 }

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
