/**
 * Hendrik Lücke-Tieke
 * dataexpedition.net
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 * 
 * Do not use without prior consent by the copyright holder.
 *
 **/

import React from 'react';
import {
  Panel
} from 'react-bootstrap';
import Icon from 'react-fontawesome';

import styles from './styles.less';

export default ({location}) => (
  <Panel className="text-center">
    <div className={styles.icon}>
      <Icon name="hand-paper-o" size="5x"/>
    </div>
    <p className="text-center lead">
        <strong>404</strong><br/><code>{location}</code>
    </p>
    <p className="text-center">
      That the requested page exists, is merely an alternative fact.
    </p>
  </Panel>
);

