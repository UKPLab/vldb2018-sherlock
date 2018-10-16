/**
 * Created by hatieke on 2017-02-05.
 */
import routeConfig from '../../config/routes';

import React from 'react';
import Json from '../JsonComponent';
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


export class PullrightFeedbackRenderer extends React.Component {
    constructor(props, children) {
        super(props, children)
    }

    static defaultProps = {
        callback: () => {
        },
        trashy: undefined,
        item: {}
    };

    __get_fo_options__(item = {item: {concept: "", value: "recommendation"}, method: "", weight: null, priority: 0}) {

        let trashVisible = "invisible";
        if (this.props.trashy !== undefined) {
            trashVisible = "";
        }
        switch (item.method) {
            case "":
            default:
                return (<span className="pull-right">
                    <FontAwesome name="question-circle"/>
                </span>);
            case "ACCEPT":
                return (<span>{item.item.concept} <span className="pull-right">
                        <FontAwesome name="plus-square-o" onClick={this.props.callback.bind(this, item, "accept")}/>
                </span></span>);
            case "ACCEPT_REJECT":
                return (<span>
                    {item.item.concept} <span className="pull-right">
                    <FontAwesome name="plus-square-o" onClick={this.props.callback.bind(this, item, "accept")}/>&nbsp;
                    <FontAwesome name="minus-square-o" onClick={this.props.callback.bind(this, item, "reject")}/>&nbsp;
                    <FontAwesome name="trash-o" className={trashVisible}
                                 onClick={this.props.callback.bind(this, item, "recommendation")}/>&nbsp;
                </span>
                </span>);
        }
    }

    render() {
        return this.__get_fo_options__(this.props.item);

    }
}
