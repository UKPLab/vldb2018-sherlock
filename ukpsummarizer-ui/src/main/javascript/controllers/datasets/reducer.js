import * as Actions from "./constants";

const INITIAL_STATE = {
    filebrowser: {
        path: "datasets/processed/DUC2004TEST",
        datasets: [],
        loaded: false
    },
    topic: {
        path: "",
        item: {},
        loaded: false
    }
};

export default function datasets(prevState = INITIAL_STATE, action) {
    switch (action.type) {
        case Actions.DATASET_LIST_REQUEST:
            return prevState;
        case Actions.DATASET_LIST_FAIL:
            console.log("failed to fetch the datasets list");
            return Object.assign({}, prevState, {
                filebrowser: {
                    loaded: false
                }
            });
        case Actions.DATASET_LIST_SUCCESS:
            return Object.assign({}, prevState, {
                filebrowser: {
                    datasets: action.entity,
                    path: action.path,
                    loaded: true
                }
            });
        case Actions.TOPIC_ITEM_REQUEST:
            return prevState;
        case Actions.TOPIC_ITEM_FAIL:
            return Object.assign({}, prevState, {
                topic: {
                    loaded: false
                }
            });
        case Actions.TOPIC_ITEM_SUCCESS:
            return Object.assign({}, prevState, {
                topic: {
                    item: action.entity,
                    path: action.path,
                    loaded: true
                }
            });
        default:
            return prevState;
    }
}
