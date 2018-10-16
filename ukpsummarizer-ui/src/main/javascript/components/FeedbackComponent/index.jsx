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
    Tooltip,
    Well,
    Popover,
    OverlayTrigger
} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import {LinkContainer} from "react-router-bootstrap";
import {ClickCycleFeedbackRenderer} from './clickcyclefeedback';
import {PopoverFeedbackRenderer} from './popoverfeedback';
import {PullrightFeedbackRenderer} from "./pullrightfeedback";

import "./style.less";

export default class FeedbackComponent extends React.Component {

    static defaultProps = {
        callback: () => {
        },
        type: "ACCEPT_REJECT",
        items: []
    };

    state = {
        renderMode: "POPOVER"
    };


    __addFeedback__(item, feedback) {
        this.props.callback([{...item.item, value: feedback}]);
    }


    render() {
        const {items, type} = this.props;

        // let sorted_descending = items;

        let feedback_items = Array.from(items)
            .sort((a, b) => {
                let av = a.priority | 0;
                let bv = b.priority | 0;

                return bv - av;
            })
            .map((e, i, a) => {
                let cn = "bg-default";
                switch (e.item.value) {
                    case "reject":
                        cn = "bg-danger";
                        break;
                    case "accept":
                        cn = "bg-success";
                        break;
                    case "recommendation":
                        cn = "";
                        break;
                }
                let po;
                switch (this.state.renderMode) {
                    case "POPOVER":
                        po = <PopoverFeedbackRenderer item={e} trashy={e.item.value !== "recommendation"}
                                                      callback={this.__addFeedback__.bind(this)}/>;
                        break;
                    case "TOGGLE":
                        po = <ClickCycleFeedbackRenderer item={e} trashy={e.item.value !== "recommendation"}
                                                         callback={this.__addFeedback__.bind(this)}/>;
                        break;
                    default:
                    case "PULLRIGHT":
                        po = <PullrightFeedbackRenderer item={e} trashy={e.item.value !== "recommendation"}
                                                        callback={this.__addFeedback__.bind(this)}/>;
                        break;

                }
                return (<li key={i} className={cn}>{po}</li>)
            });

        const makeBtn = (mode, tooltiptext) => {
            const tooltip = <Tooltip id="feedback-rendermode-selector-tooltip">{tooltiptext}</Tooltip>
            if (this.state.renderMode === mode) {

                return (<OverlayTrigger key={mode} placement="top" overlay={tooltip}>
                    <Button active>{mode}</Button>
                </OverlayTrigger>);
            } else {
                return (<OverlayTrigger key={mode} placement="top" overlay={tooltip}>
                    <Button onClick={this.__toggle_rendermode.bind(this, mode)}>{mode}</Button>
                </OverlayTrigger>);
            }
        };

        let modes = [];
        modes.push(makeBtn("POPOVER", "In this mode, you can click each text snippet to make a popup appear. In that popup, you then can select to accept, reject the selected concept, or remove your previous tag."));
        modes.push(makeBtn("PULLRIGHT", "In this mode, you select to accept, reject the selected concept, or remove your previous tag by clicking the '+','-' or click the trash can."));
        modes.push(makeBtn("TOGGLE", "In this mode, you can click each text snippet to make a popup appear. In that popup, you then can select to accept, reject the selected concept, or remove your previous tag."));


        let divClasses = "list-unstyled ";
        switch (this.state.renderMode) {
            case "POPOVER":
            default:
                divClasses = divClasses + " list-inline orlist";
                break;
            case "PULLRIGHT":
                divClasses = divClasses + " columnsmulti";
                break;
            case "TOGGLE":
                divClasses = divClasses + "list-inline orlist";
        }
        let rendermodeselector = <ButtonGroup>{modes}</ButtonGroup>;

        return (<Grid fluid>
                <Row>
                    Toggle rendermode: <span className="pull-right block">{rendermodeselector}</span>
                </Row>
                <Row>
                    <ul className={divClasses}>
                        {feedback_items}
                    </ul>
                </Row>
            </Grid>
        );
    }

    __toggle_rendermode(mode) {
        this.setState({renderMode: mode});
    }
}
