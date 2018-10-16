import * as Actions from "./constants";

const INITIAL_STATE = {
    summary: {
        state: "NEW",
        data: ["No summary computed yet."]
    },
    weights: {
        state: "NEW",
        data: {"no weights": 1}
    },
    interactions: {
        state: "NEW",
        data: []
    }
};

export default function summary(prevState = INITIAL_STATE, action) {
    switch (action.type) {
        case Actions.RESET_STORE:
            return Object.assign({}, INITIAL_STATE);
        case Actions.SUMMARY_LOAD_REQUEST:
            return Object.assign({}, prevState, {
                summary: {
                    data: [],
                    state: "LOADING"
                }
            });

            return prevState;
        case Actions.SUMMARY_LOAD_FAIL:
            return Object.assign({}, prevState, {
                summary: {
                    data: [],
                    state: "FAILED"
                }
            });
        case Actions.SUMMARY_LOAD_SUCCESS:
            return Object.assign({}, prevState, {
                summary: {
                    data: action.entity.summary,
                    state: "LOADED"
                }
            });
        case Actions.RECOMMENDATION_LOAD_REQUEST:
            return Object.assign({}, prevState, {
                interactions: {
                    data: [],
                    state: "LOADING"
                }
            });

            return prevState;
        case Actions.RECOMMENDATION_LOAD_FAIL:
            return Object.assign({}, prevState, {
                interactions: {
                    data: [],
                    state: "FAILED"
                }
            });
            return prevState;
        case Actions.RECOMMENDATION_LOAD_SUCCESS:
            return Object.assign({}, prevState, {
                interactions: {
                    data: action.payload,
                    state: "LOADED"
                }
            });
            return prevState;
        // return Object.assign({}, prevState, {
        //     recommendations: {
        //         iteration: action.entity.iteration,
        //         summary: action.entity.summary,
        //         weights: action.entity.weights,
        //         data: prevState.interactions.recommendations,
        //         loaded: true
        //     }
        // });

        default:
            return prevState;
    }
}
