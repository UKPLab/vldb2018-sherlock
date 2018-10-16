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

// mvc stuff
import {connect} from 'react-redux';
import {bindActionCreators} from 'redux';
import * as DatasetActions from '../../controllers/datasets/actions';
import routeConfig from '../../config/routes';

// ui stuff
import {
    Grid,
    Row,
    Col,
    Table,
    Button,
    ButtonGroup,
    Jumbotron,
    Breadcrumb
} from 'react-bootstrap';
import {LinkContainer} from "react-router-bootstrap";

import FontAwesome from 'react-fontawesome';


class FileBrowser extends React.Component {
    constructor(props) {
        super(props);
    }

    componentDidMount() {
        window.scrollTo(0, 0);
        const {filebrowser: {path, datasets}} = this.props;
        this.__loadDatasetPath(path);
    }

    componentWillReceiveProps(nextProps) {

    }

    componentWillUpdate(nextProps, nextState) {

    }

    __loadDatasetPath(path = "/") {
        this.props.actions.getDatasets(path);
    }

    render() {
        console.log("Rendering FileBrowser");
        const {filebrowser: {path, datasets}} = this.props;

        let rows = datasets.map(x => {
            var identifier, type;

            if (x.type === "DIRECTORY") {
                identifier = (<b><a onClick={this.__loadDatasetPath.bind(this, path + "/" + x.name)}>{x.name}</a></b>);
            } else {
                identifier = (<b>{x.name}</b>);
            }

            let taskDesc = null;
            let narrative="";
            let topic = null;
            if (x.task) {
                identifier = (<LinkContainer to={{pathname: routeConfig.assignment, query: {topic: path + "/" + x.name}}}>
                    <Button ><FontAwesome name="play"/>{x.name}</Button>
                </LinkContainer>);

                taskDesc = x.task.title;
                narrative=x.task.narrative;
            }

            return (<tr key={x.name}>
                    <td>{identifier}{topic}</td>
                    <td>{taskDesc}</td>
                    <td>{narrative}</td>
                    {/*<td>{x.numberOfDocuments}</td>*/}
                    {/*<td>{x.numberOfModels}</td>*/}
                </tr>
            )
        });


        let datasetTable = (<Table striped bordered hover>
            <thead>
            <tr>
                <th>id</th>
                <th>Task description (if available)</th>
                <th>Narrative</th>
                {/*<th>Number of documents</th>*/}
                {/*<th>Number of models</th>*/}
            </tr>
            </thead>
            <tbody>
            {rows}
            </tbody>
        </Table>);

        const breadcrumbInstance = (<Breadcrumb>
            <Breadcrumb.Item onClick={this.__loadDatasetPath.bind(this, "/")}>
                Home
            </Breadcrumb.Item>
            {path.split("/")
                .filter(s => "" !== s)
                .map((v, i, a) => {
                    const pathVar = a.slice(0, i + 1).join("/");
                    return (<Breadcrumb.Item key={pathVar} onClick={this.__loadDatasetPath.bind(this, pathVar)}>
                        {v}
                    </Breadcrumb.Item>);
                })}
        </Breadcrumb>);

        return (
            <Grid>
                <Row>
                    <Col xs={12}>
                            <h1>File Browser</h1>
                            <p>Browse the repository.</p>
                    </Col>
                </Row>
                <Row>
                    <Col xs={12}>
                        <p className="lead">Browse the file system, start to play with a topic.</p>
                    </Col>
                    <Col xs={12}>
                        {breadcrumbInstance}
                        {datasetTable}
                        <Button><FontAwesome name="magic"/> Start the wizard for a new data
                            set</Button>
                    </Col>
                </Row>
            </Grid>
        );
    }
}

/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = (state) => {
    const {datasets: {filebrowser}} = state;
    return {
        filebrowser
    };
};

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */

function mapDispatchToProps(dispatch) {
    return {
        actions: bindActionCreators(Object.assign({}, DatasetActions), dispatch)
    }
}

export default connect(mapStateToProps, mapDispatchToProps)(FileBrowser);


/** WEBPACK FOOTER **
 ** ./src/main/javascript/containers/FileBrowser/index.jsx
 **/
