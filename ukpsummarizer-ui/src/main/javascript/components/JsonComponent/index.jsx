/**
 * Created by hatieke on 2017-02-05.
 */


import React from 'react';
import JSONPretty from 'react-json-pretty';
import './solarized-light.styl';


export default ({json}) => (<JSONPretty json={json}></JSONPretty>)