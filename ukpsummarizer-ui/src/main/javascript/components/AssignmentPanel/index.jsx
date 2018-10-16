import React from 'react';
import {Panel, Media, Thumbnail, Button} from 'react-bootstrap';
import Json from 'react-json-pretty';
import FontAwesome from 'react-fontawesome';

export default class AssignmentPanel extends React.Component {

    static defaultProps = {
        callback : () => {}
    };

    constructor(props) {
        super(props);
    }

    render() {
        const e = this.props.assignment;
        const iteration = e.iterations.reduce((x,y) => y.iteration > x.iteration?y:x);

        const header = "lalala";
        const style = e.active ? "bg-info" : "";


        const progress = e.iterations.length;

        const historyItems = e.iterations.map((item, idx, arr) => {
            const numAccept = item.interactions.filter(e => e.value === "accept").length;
            const numReject = item.interactions.filter(e => e.value === "reject").length;

            const total = item.interactions.length;
            return <li key={idx}> accepted {numAccept},  rejected {numReject}, total {total}.</li>
        });

        return (<Media className={style}>
            <Media.Left>
                <h1>#{e.id}</h1>
            </Media.Left>
            <Media.Body>
                <Media.Heading componentClass="h2"><small>{e.title}:</small> {e.narrative}
                {e.run_id}
                    <span className="pull-right">
                    <Button onClick={this.props.callback.bind(this, e.id)}>
                        <FontAwesome name="play-circle-o" fixedWidth size="4x"/>
                    </Button>
                </span>
                </Media.Heading>
                <p className="lead">You have already done {progress} iterations. Your history so far:</p>
                <ol>
                    {historyItems}
                </ol>
                <p>{iteration.summary.join(" ")}</p>
            </Media.Body>
        </Media>);
    }
}
