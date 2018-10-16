import React from 'react';
import MultiMap from "../../models/MultiMap";


export default class TopicCollectionComponent extends React.Component {

    static defaultProps = {
        content: []
    };

    render() {

        const {content} = this.props;

        const mm = content.reduce((h,t) => {
            h.add(t.doc_id, t);
            return h;
        }, new MultiMap());


        let docs = [];
        for(const e of mm.entries()) {
            docs.push(<Document key={e[0]} id={e[0]} sentences={e[1]}/>);
        }

        return (<div>
            <h1>{mm.size()} documents</h1>
            {docs}
        </div>)
    }

}


Document = ({sentences, id="-1"}) => {
    return (<div key={id}>
        <h4>Document {id}</h4>
        <p>{sentences.sort((a,b) => (a.sent_id - b.sent_id)).map(s => (<span key={s.sent_id}>{s.untokenized_form}<br/></span>))}</p>

    </div>);
};
