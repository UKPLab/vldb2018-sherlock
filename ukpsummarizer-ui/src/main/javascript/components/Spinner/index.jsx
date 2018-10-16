import React from 'react';
import {
    Grid,
    Row,
    Col,
    Button,
    ButtonGroup,
    Jumbotron,
    Panel,
    PageHeader,
    Well
} from 'react-bootstrap';
import {LinkContainer} from "react-router-bootstrap";

import FontAwesome from 'react-fontawesome';

export default ({text = "Rockin'..."}) => {
    return (<Well className="text-center">
            <FontAwesome name="circle-o-notch" fixedWidth spin/><br/>
            {text}
        </Well>);
}
