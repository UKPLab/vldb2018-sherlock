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
                    <td>{x.numberOfDocuments}</td>
                    <td>{taskDesc}</td>
                    <td>{narrative}</td>
                    {/*<td>{x.numberOfDocuments}</td>*/}
                    {/*<td>{x.numberOfModels}</td>*/}
                </tr>
            )
        }
        });


        let datasetTable = (<Table striped bordered hover>
            <thead>
            <tr>
                <th>Dataset</th>
                <th>Documents</th>
                <th>Task Description</th>
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
                            <h2> Welcome </h2>
                            <p> This is a demo of the Interactive Summarization of Large Text Collections presented at VLDB 2018</p> <br/>
                            <p> Browse the document collections</p>
                    </Col>
                </Row>
                <Row>
                    <Col xs={12}>
                        {/*breadcrumbInstance*/}
                        {datasetTable}
                        <Button><FontAwesome name="magic"/> Add a new dataset</Button>
                    </Col>
                </Row>
                <Row>
                <br/><br/><br/><br/>
                <p>
                This demo is supported by the DFG research training group <a href='https://www.aiphes.tu-darmstadt.de/de/aiphes/'> AIPHES (Adaptive Information Preparation from Heterogeneous Sources)</a>, <a href="https://www.informatik.tu-darmstadt.de/ukp/ukp_home/">UKP Lab, Technische Universität Darmstadt </a>
                and <a href='http://binnig.name/'> Data Management Lab, Technische Universität Darmstadt</a>.
                </p>
                <p>
                For questions, please contact <a href='https://www.informatik.tu-darmstadt.de/ukp/ukp_home/staff_ukp/detailseite_mitarbeiter_1_41216.en.jsp'>Avinesh PVS</a>
                <br/><br/><br/><br/><br/><br/><br/><br/>
                </p>

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
