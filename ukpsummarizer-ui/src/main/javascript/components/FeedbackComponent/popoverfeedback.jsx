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


export class PopoverFeedbackRenderer extends React.Component {
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
        const {item, trashy} = this.props;

        let trashVisible = "invisible";
        if (trashy !== undefined) {
            trashVisible = "";
        }

        let buttons = null;

        switch (item.method) {
            case "":
            default:
                buttons = (<ButtonGroup>
                    <Button><FontAwesome name="question-circle"/></Button>
                </ButtonGroup>);
            case "ACCEPT":
                buttons = (<ButtonGroup>
                    <Button><FontAwesome size='2x' name="pencil-square" className="accept"
                                         onClick={this.props.callback.bind(this, item, "accept")}/>
                    </Button>
                </ButtonGroup>);
            case "ACCEPT_REJECT":
                buttons = (<ButtonGroup>
                    <Button className="accept">
                        <FontAwesome size='2x' name="thumbs-o-up"
                                     onClick={this.props.callback.bind(this, item, "accept")}/>
                    </Button>
                    <Button  className="reject">
                        <FontAwesome size='2x' name="thumbs-o-down"
                                     onClick={this.props.callback.bind(this, item, "reject")}/>
                    </Button>
                    <Button>
                        <FontAwesome size='2x' name="eraser" className={trashVisible}
                                     onClick={this.props.callback.bind(this, item, "recommendation")}/>
                    </Button>
                </ButtonGroup>);

                const popoverClickRootClose = (
                    <Popover id="popover-trigger-click-root-close" title={"Feedback for " + item.item.concept}>
                        {buttons}
                    </Popover>
                );


                let poContent = <ButtonGroup>{buttons}</ButtonGroup>;


                return (<OverlayTrigger trigger={["click"]} rootClose overlay={popoverClickRootClose}>
                    <span
                        style={{"fontSize": ((Math.log(item.weight) / Math.log(10)) + 1) + "em"}}>{item.item.concept}</span>
                </OverlayTrigger>);


        }

    }
}
