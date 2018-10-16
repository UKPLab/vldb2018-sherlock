import * as Actions from "./constants";

const INITIAL_STATE = new Map();

/**
 *
 * @param prevState
 * @param action should consist of an payload.id and an payload.data or payload.error
 * @returns {*}
 */
export default function assignmentTemplates(prevState = INITIAL_STATE, action) {
    switch (action.type) {
        case Actions.LOAD_ASSIGNMENT_TEMPLATE_REQUEST:
            return new Map(prevState).set(action.payload.id, {state: "LOADING", data: {}});
        case Actions.LOAD_ASSIGNMENT_TEMPLATE_SUCCESS:
            // return new Map(prevState).set(action.payload.id, {state: "LOADED", data: action.payload});
        // TODO replace line 16 with this:
        return new Map(prevState).set(action.payload.id, {state: "LOADED", data: action.payload.data});
        case Actions.LOAD_ASSIGNMENT_TEMPLATE_FAIL:
            return new Map(prevState).set(action.payload.id, {state: "FAILED", data: {}, error: action.payload.error});
        case Actions.LOAD_ASSIGNMENT_TEMPLATES_REQUEST:
            return prevState;
        case Actions.LOAD_ASSIGNMENT_TEMPLATES_FAIL:
            return prevState;
        case Actions.LOAD_ASSIGNMENT_TEMPLATES_SUCCESS:
            const newState = new Map(prevState);
            action.payload.forEach(minimalEntry => {
                if (newState.has(minimalEntry.id)) {
                    if (newState.get(minimalEntry.id).state === "FAILED") {
                        newState.set(minimalEntry.id, {state: "MINIMAL", data: minimalEntry});
                    }
                } else {
                    newState.set(minimalEntry.id, {state: "MINIMAL", data: minimalEntry});
                }
            });
            return newState;


        default:
            return prevState;
    }
}


