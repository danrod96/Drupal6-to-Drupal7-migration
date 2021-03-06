<?php
/**
 * @file
 *
 *
 * Webservice user related methods: creation, update user profile, logging on, logging off, etc.
 *
 * Version 1.0 - 13/01/2015
 * - Initial version
 *
 */

  /**
  * Function existeUsuario
  *
  * Validates if username exists in Drupal
  *
  */
 function existeUsuario($user)
 {
    global $db_prefix;

    //users are stored in another DB
  	$db_users = $db_prefix["users"];

  	if (!isset($db_sessions)){
  		$db_users = "";
  	}

 	  $response = array("codigo" => "0","msg" => "No existe usuario");

    //check if user exists
  	$query_validar_correo = "SELECT uid FROM {" . $db_users . "users} WHERE name = '%s'";

  	$es_correo = db_fetch_object(db_query($query_validar_correo, $user));

  	if ($es_correo->uid){
  		$response = array("codigo" => "1","msg" => "Existe usuario");
  	  	return json_encode($response);
  	}

  	$query_validar_correo2 = "SELECT uid FROM {" . $db_users . "users} WHERE mail = '%s'";

  	$es_correo2 = db_fetch_object(db_query($query_validar_correo2, $user));

  	if ($es_correo2->uid){
  	 $response = array("codigo" => "1","msg" => "Existe usuario");
  	 return json_encode($response);
  	}

 	 return json_encode($response);
 }

 /**
  * FUnction crearUsuario
  *
  * Create User in Drupal
  */
 function crearUsuario($correo, $usuario, $contrasena, $nombre, $apellido, $telefono, $tipodoc, $numdoc)
 {
 	$response = array();

 	 //basic user fields
 	 $campos_basicos = array(
 	 	'name' => $usuario,
 	 	'mail' => $correo,
 	 	'pass' => $contrasena,
 	 	'status' => 1,
 	 );

 	 //creating user
 	 $cuenta = user_save("", $campos_basicos);

 	 if (!$cuenta){
 	 	$response = array("codigo" => "0","msg" => "No se creo el usuario");
 	 	return json_encode($response);
 	 }

 	 if ($tipodoc == "2323"){
 	 	 $newtipodoc = "NIT";
 	 } else if ($tipodoc == "2322"){
 	 	$newtipodoc = "Cédula de ciudadanía";
 	 } else if ($tipodoc == "2324"){
 	 	$newtipodoc = "Cédula de extranjería";
 	 } else {
 	 	$newtipodoc = "Cédula de ciudadanía";
 	 }

   //profile information: first name, last name, etc
 	 $edit = array(
 	 	'profile_id_number' => $numdoc,
 	 	'profile_name' => $nombre,
 	 	'profile_phone' => $telefono,
 	 	'profile_pwd_plain_text' => $contrasena,
 	 	'profile_lastname' => $apellido,
 	 	'profile_id_type' => $newtipodoc
 	 );

 	 profile_save_profile($edit, $cuenta, 'Datos del Usuario', TRUE);

 	 $response = array("codigo" => "1","msg" => "$cuenta->uid");

 	 return json_encode($response);
 }

 /**
  * Function isLogged
  *
  * Checks if user is logged
  * INPUT: sid
  */
 function isLogged($cookie_session_value)
 {
  global $db_prefix;

  //user DB
  $db_sessions = $db_prefix["sessions"];

  if (!isset($db_sessions)){
  	$db_sessions = "";
  }

 	$response = array("codigo" => "0","msg" => "No esta logueado");

 	if (empty($cookie_session_value)){
 		return json_encode($response);
 	}

 	//get uid from sid
 	$query_get_session = "SELECT uid, sid FROM {" . $db_sessions . "sessions} WHERE sid = '%s'";
 	$info_session = db_fetch_object(db_query($query_get_session, $cookie_session_value));

 	if ($info_session->uid == 0){
 		return json_encode($response);
 	} else if ($info_session->uid > 0 && !is_null($info_session->sid)){
 		$response = array("codigo" => "1", "msg" => "Esta logueado");
 		return json_encode($response);
 	} else if (is_null($info_session)){
 		return json_encode($response);
 	} else {
 		return json_encode($response);
 	}

 }

  /**
  * Function getSessionNme
  *
  * Returns session name
  *
  */
 function getSessionName()
 {
   	return session_name();
 }

  /**
  * Function logIn
  *
  * User log in. Input: mail and password
  */
  function logIn($correo_ext, $contrasena)
  {
  	$response = array("codigo" => "-1", "msg" => "No hay nombre de usuario asociado al correo o no existe usuario");

  	global $cookie_domain;
  	global $user;
    global $db_prefix;

   	//user DB
  	$db_sessions = $db_prefix["sessions"];

  	if (!isset($db_sessions)){
  	 $db_sessions = "";
  	}

    //get name uid from mail
	  $query_obtener_usuario = "SELECT name,uid FROM {" . $db_sessions . "users} WHERE name = '%s'";
	  $usuario = db_fetch_object(db_query($query_obtener_usuario, $correo_ext));

	  if (is_null($usuario->name)){
	  	  $query_obtener_usuario2 = "SELECT name,uid FROM {" . $db_sessions . "users} WHERE mail = '%s'";
	  	  $usuario2 = db_fetch_object(db_query($query_obtener_usuario2, $correo_ext));

		  if (is_null($usuario2->name)){
		  	return json_encode($response);
		  }
	  }

	  $user = user_load($usuario->uid);

	  $params = array(
	  					'name' => $usuario->name,
	  					'pass' => $contrasena,
	  );

    //start user authentication
	  $info_usuario = user_authenticate($params);

	  if($info_usuario->uid == 0){
	  	//wrong user/pass
	  	$response = array("codigo" => "0", "msg" => "Credenciales incorrectas");
	  	return json_encode($response);
	  } else {
	  	$response = array("codigo" => "1", "uid" => $info_usuario->uid, "timestamp" => $info_usuario->login, "msg" => "Usuario logueado");
	  	return json_encode($response);
	  }
  }

 /*
  * Function buildCookie
  *
  * Returns SID string
  */
 function buildCookie($uid, $login)
 {
    global $db_prefix;

   	//user DB
  	$db_sessions = $db_prefix["sessions"];

  	if (!isset($db_sessions)){
  		$db_sessions = "";
    }

    $query_obtener_sid = "SELECT sid FROM {" . $db_sessions . "sessions} WHERE uid = %d AND timestamp = %s";
    $sid_session = db_fetch_object(db_query($query_obtener_sid, $uid, $login));

    if(is_null($sid_session->sid)) {
	   $response = array("codigo" => "0", "msg" => "Sesion no iniciada");
    } else {
	   $response = array("codigo" => "1", "sid" => $sid_session->sid, "msg" => "Sesion iniciada");
    }

    return json_encode($response);
 }

  /**
  * Function logOut
  *
  * Logs Out User
  */
 function logOut($cookie_session_value)
 {
 	global $user;

 	$response = array("codigo" => "1", "msg" => "Usuario deslogueado");

 	$tmp = NULL;

 	sess_destroy_sid($cookie_session_value);

  //invokes hook_user() with the logout operation
 	esinergia_exporter_user('logout', $tmp, $user);

 	return json_encode($response);
 }

  /**
  * FUnction getUid
  *
  * Input: Session ID String
  * Output: UID
  */
 function getUid($cookie_session_value)
 {
    global $db_prefix;

   	//user DB
  	$db_sessions = $db_prefix["sessions"];

  	if (!isset($db_sessions)){
  		$db_sessions = "";
  	}

  	if (!isLogged($cookie_session_value)){
 		  return NULL;
 	  }

 	  $query_get_uid_session = "SELECT uid FROM {" . $db_sessions . "sessions} WHERE sid = '%s'";
 	  $uid_session = db_fetch_object(db_query($query_get_uid_session, $cookie_session_value));

 	  if (is_null($uid_session->uid)){
 		 return NULL;
 	  } else {
 		 return ($uid_session->uid);
 	  }
 }

 /**
  * Function getUserName
  *
  * Input: Session ID String
  * Output: User Name
  */
 function getUserName($cookie_session_value)
 {
  global $db_prefix;

  //user DB
  $db_users = $db_prefix["users"];

  if (!isset($db_users)){
    $db_users = "";
  }

  if (!isLogged($cookie_session_value)){
    return NULL;
  }

  $uid_session = getUid($cookie_session_value);

  $query_username_session = "SELECT name FROM {" . $db_users . "users} WHERE uid = %d";
  $username_session = db_fetch_object(db_query($query_username_session, $uid_session));

  if (is_null($username_session->name)){
    return NULL;
  } else {
    return ($username_session->name);
  }

 }

  /**
   * Function registerUser
   *
   * Registers Users in Drupal 6
   *
   * Input: User Info: email, names, password, ID
   * Output: SUCCESS or ERROR
   */
   function registerUser($email, $tipodoc, $numdoc, $nombres, $apellidos, $telefono, $password)
   {
      global $db_prefix;

      //user DB
      $db_users = $db_prefix["users"];

      if (!isset($db_users)){
        $db_users = "";
      }

      //user profile DB
      $db_profiles = $db_prefix["profile_values"];

      if (!isset($db_profiles)){
        $db_profiles = "";
      }


      $response = array("codigo" => "-1", "msg" => "El usuario o correo ya existe");

      $user_name = $email;

      $query_validar_correo = "SELECT uid FROM {" . $db_users . "users} WHERE mail = '%s'";
      $es_correo = db_fetch_object(db_query($query_validar_correo, $email));

      if ($es_correo->uid){
         return json_encode($response);
      }

      $query_validar_correo2 = "SELECT uid FROM {" . $db_users . "users} WHERE name = '%s'";
      $es_correo2 = db_fetch_object(db_query($query_validar_correo2, $email));

      if ($es_correo2->uid){
        return json_encode($response);
      }

      $query_validar_cedula = "SELECT value FROM {" . $db_profiles . "users} WHERE fid = 2 AND value = '%s'";
      $es_cedula = db_fetch_object(db_query($query_validar_cedula, $numdoc));

      if ($es_cedula->value){
        return json_encode($response);
      }

      if ($tipodoc == "1"){
        $newtipodoc = "2322";
      } else if ($tipodoc == "2"){
        $newtipodoc = "2323";
      } else if ($tipodoc == "3"){
        $newtipodoc = "2324";
      } else {
        $newtipodoc = "2322";
      }

      //create user in Drupal 6
      $res = crearUsuario($email, $email, $password, $nombres, $apellidos, $telefono, $tipodoc, $numdoc);

      $tmp = json_decode($res);

      if (!$tmp->codigo){
        return $res;
      } else {
        //create user in D7 just in case
        $client = new SoapClient("https://seg.eltiempo.com/publicaraviso/sites/all/modules/custom/converter_etc/wsdl/converter.wsdl");

        $res2 = $client->crearUsuario($email, $email, md5($password), $nombres, $apellidos, $telefono, $newtipodoc, $numdoc, $tmp->msg);

        return $res2;
      }
   }

 /**
  * Function resetPasswd
  *
  * Resets password user
  */
  function resetPasswd($cadena)
  {
      global $db_prefix;

      //user DB
      $db_users = $db_prefix["users"];

      if (!isset($db_users)){
        $db_users = "";
      }

      $response = array("codigo" => "-1", "msg" => "No se cambio el password correctamente");
      global $user;

      $query_validar_correo = "SELECT uid FROM {" . $db_users . "users} WHERE mail = '%s'";
      $es_correo = db_fetch_object(db_query($query_validar_correo, $cadena));

      $query_validar_usuario = "SELECT uid FROM {" . $db_users . "users} WHERE name = '%s'";
      $es_usuario = db_fetch_object(db_query($query_validar_usuario, $cadena));

      if (is_null($es_correo->uid) && is_null($es_usuario->uid)){
        return json_encode($response);
      } else {

        if ($es_correo->uid){
          $correo_usuario = $cadena;

          $user = user_load($es_correo->uid);
        } else {
          $query_obtener_correo = "SELECT mail,uid FROM {" . $db_users . "users} WHERE name = '%s'";

          $tmp = db_fetch_object(db_query($query_obtener_correo, $cadena));
          $correo_usuario = $tmp->mail;

          $user = user_load($tmp->uid);
        }

        //send password reset email
        if(_user_mail_notify('password_reset', $user, $language = NULL)){
          $response = array("codigo" => "1", "msg" => "Se cambio el password correctamente");
        return json_encode($response);
        } else {
          return json_encode($response);
        }

    }
  }
?>