<?php
/**
 * @file
 * 
 * Webservice Methods 
 *
 * Version 1.0 - 10/02/2014
 * - Initial version
 * 
 * Version 1.5 - 20/03/2014
 * - Security Settings
 * 
 * Version 1.7 - 23/03/2014
 * - logOut Definition
 * 
 * Version 1.8 - 02/07/2014
 * - logIn Definition
 * 
 * Version 1.9 - 06/07/2014
 * - resetPasswd Definition
 * - registerUser Definition
 * - Extra security validations for all methods
 *
 * Version 2.0 - 30/07/2014
 * - Bugfixes suggested for our client
 * - Rules for password generation
 * 
 * Version 2.1 - 03/08/2014
 * - logIn bugfixes for session persistence.
 */
 
 /**
  * Function isLogged
  *
  * Checks if user is logged
  */
 function isLogged($cookie_session_value)
 {
  $response = array("codigo" => "0","msg" => "No esta logueado");
  
  if (empty($cookie_session_value)){
    return json_encode($response);
  }
    
  $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

  $res = $client->isLogged($cookie_session_value);

  return $res;
 }
 
  /**
  * Function getUserName
  * 
  * returns user name
  */
 function getUserName($cookie_session_value)
 {  
  $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

  $res = $client->getUserName($cookie_session_value);

  return $res;
 }
 
  /**
  * FUnction getUid
  *
  * Input: Session ID String
  * Output: UID
  */
 function getUid($cookie_session_value)
 {
  $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

  $res = $client->getUid($cookie_session_value);

  return $res;
 }
 
  /**
  * Function logOut
  *
  * Logs Out User
  */
 function logOut($cookie_session_value)
 {
  $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

  $res = $client->logOut($cookie_session_value);

  //desloguear en Publicacion D7 tambien
  $uid6 = getUid($cookie_session_value);

  $client2 = new SoapClient("https://seg.eltiempo.com/publicaraviso/sites/all/modules/custom/converter_etc/wsdl/converter.wsdl");

  $client2->logOut7($uid6);

  return $res;
 }
 
 /**
  * Function logIn
  *
  * User log in. Input: mail and password
  *
  */
  function logIn($correo_ext, $contrasena)
  {  
   $response = array("codigo" => "-1", "msg" => "No hay nombre de usuario asociado al correo o no existe usuario");

   $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

   $res = $client->logIn($correo_ext, $contrasena);

   $tmp = json_decode($res);

   if ($tmp->codigo  == -1){
      return json_encode($response);
    } else if ($tmp->codigo == 0){
      $response = array("codigo" => "0", "msg" => "Credenciales incorrectas");
      return json_encode($response);
    } else {
      $res2 = $client->buildCookie($tmp->uid, $tmp->timestamp);

      $tmp2 = json_decode($res2);

      $response = array("codigo" => $tmp2->sid, "msg" => "Usuario logueado");

      //loguear a Drupal 7
      $client2 = new SoapClient("https://seg.eltiempo.com/publicaraviso/sites/all/modules/custom/converter_etc/wsdl/converter.wsdl");

      $sessname = $client2->getSessionName();

      $res4 = $client2->logIn7($correo_ext, $contrasena);

      $tmp4 = json_decode($res4);

      //logueo correcto en D7, construir cookie
      if ($tmp4->codigo == 1){
        $res5 = $client2->buildCookie($tmp4->uid, $tmp4->timestamp);

        $tmp5 = json_decode($res5);
      } 

      //generar la nueva respuesta
      $response = array("codigo" => $tmp2->sid, "msg" => $tmp5->codigo);

      return json_encode($response);
    }
  }
  
 /**
  * Function resetPasswd
  *
  * Resets password user
  */
  function resetPasswd($cadena)
  {
    $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

    $res = $client->resetPasswd($cadena);

    return $res;
  }
  
  /**
   * Function registerUser
   *
   * Registers Users
   *
   * Input: User Info: email, names, password, ID
   * Output: SUCCESS or ERROR
   */
   function registerUser($email, $tipodoc, $numdoc, $nombres, $apellidos, $telefono, $password)
   {
     $client = new SoapClient("http://clasificados.eltiempo.com/sites/all/modules/custom/esinergia_exporter/wsdl/exporter.wsdl");

     $res = $client->registerUser($email, $tipodoc, $numdoc, $nombres, $apellidos, $telefono, $password);

     return $res;
   }