import React from 'react';

import {Button, ButtonGroup, Collapse, Fade, ListGroup, ListGroupItem} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';


export default class SubmitControlPanel extends React.Component {


    render() {

        const {interactions, weights, callback} = this.props;
        const i = [...interactions.entries()].map(([key, value]) => ({key, value})).map(e => ({
            weight: Math.log((weights[e.key] + 1) / Math.log(10)) + 1 | 1.0,
            ...e.value
        })).map(e => {
            const {value, concept, concept: k, weight} = e;

            return <Item key={concept} value={value} concept={concept} weight={weight} callback={callback}/>;
        }).sort((a, b) => {
            return (a.key < b.key) ? -1 : (a.key === b.key) ? 0 : 1;
        });

        if (i.length === 0) {
            return <ListGroup><ListGroupItem><em>No feedback gathered yet.</em></ListGroupItem></ListGroup>;
        }
        return <ListGroup>{i}</ListGroup>;
    }
}

export class Item extends React.Component {

    state = {
        value: "recommendation"
    };

    constructor(props, ...args) {
        super(props, ...args);
        this.callbackHandler = this.callbackHandler.bind(this);
    }

    componentWillReceiveProps(nextProps) {
        this.setState({
            value: nextProps.value
        })
    }

    callbackHandler() {
        this.props.callback(this.props.concept, this.state.value);
    }

    render() {
        const {value, concept, weight, callback} = this.props;

        const cn = (value === "reject") ? "danger" : (value === "accept") ? "success" : "";
        return (<ListGroupItem bsStyle={cn}>
            <span style={{"fontSize": weight + "em"}}>{concept}</span>
            <ButtonGroup className="pull-right">
                <Button bsSize="xsmall" className={(value === "accept") ? "invisible" : ""}>
                    <FontAwesome name="thumbs-o-up"
                                 onClick={() => {
                                     this.setState({
                                         value: "accept"
                                     }, this.callbackHandler);
                                 }}/>
                </Button>
                <Button bsSize="xsmall" className={(value === "reject") ? "invisible" : ""}>
                    <FontAwesome name="thumbs-o-down"
                                 onClick={() => {
                                     this.setState({
                                         value: "reject"
                                     }, this.callbackHandler);
                                 }}/>
                    {/*// onClick={this.callbackHandler.bind(this, k, "reject")}/>*/}
                </Button>
                <Button bsSize="xsmall" className={(value === "recommendation") ? "invisible" : ""}>
                    <FontAwesome name="eraser"
                                 onClick={() => {
                                     this.setState({value: "recommendation"}, this.callbackHandler);
                                 }}/>
                    {/*// onClick={this.callbackHandler.bind(this, k, "recommendation")}/>*/}
                </Button>
            </ButtonGroup>
        </ListGroupItem>);
    }
}
