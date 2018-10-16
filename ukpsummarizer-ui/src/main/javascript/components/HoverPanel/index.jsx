import React from 'react';
import {
    Panel
} from 'react-bootstrap';

import styles from './styles.less';

export default ({children}) => (<Panel className={styles.hoverable}>
    {children}
</Panel>);
