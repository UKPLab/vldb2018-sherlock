import React from 'react';
import {Button, Jumbotron, Panel} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import Json from '../JsonComponent';
import {LinkContainer} from "react-router-bootstrap";
import routeConfig from '../../config/routes';

export default class AssignmentTemplateComponent extends React.Component {

    constructor(...args) {
        super(...args);
        this.activateAssignment = this.activateAssignment.bind(this);
    }

    activateAssignment() {
        this.props.activateAssignment(this.props.template.data.topic);
    }

    render() {
        const {template} = this.props;


        return (<Panel style={{minHeight: "200px"}}>
            <h3>{template.data.title}</h3>
            {/*<h3>{template.data.id}</h3>*/}
            {/*<Json json={template}/>*/}
            <p>
                <LinkContainer to={{pathname: routeConfig.assignment, query: {topic: template.data.topic}}}>
                    <Button bsStyle="primary"><FontAwesome name="play" fixedWidth/>Show details</Button>
                </LinkContainer>
            </p>
        </Panel>);
    }

}
