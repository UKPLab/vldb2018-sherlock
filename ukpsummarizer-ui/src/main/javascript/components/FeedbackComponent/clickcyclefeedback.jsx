/**
 * Created by hatieke on 2017-02-05.
 */
import routeConfig from '../../config/routes';

import React from 'react';
import Json from 'components/JsonComponent';
import {
    Grid,
    Row,
    Col,
    Button,
    ButtonGroup,
    Jumbotron,
    Well,
    Popover,
    OverlayTrigger
} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import {LinkContainer} from "react-router-bootstrap";


export class ClickCycleFeedbackRenderer extends React.Component {
    constructor(props, children) {
        super(props, children)
    }

    static defaultProps = {
        callback: () => {
        },
        trashy: undefined,
        item: {}
    };


    render() {
        const trashVisible = false;
        const {item, callback} = this.props;

        switch(item.item.value) {
            default:
            case "recommendation":
                return (<span onClick={callback.bind(this, item, "reject")}>{item.item.concept}</span>);
            case "reject":
                return (<span onClick={callback.bind(this, item, "accept")}>{item.item.concept}</span>);
            case "accept":
                return (<span onClick={callback.bind(this, item, "recommendation")}>{item.item.concept}</span>);
            }

    }
}
