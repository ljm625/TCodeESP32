const modalTemplate = document.createElement("template");
modalTemplate.innerHTML = `
<style>
    .modal-hidden {
        display: none !important;
    }
    .modal-overlay {
        display: flex;
        position: fixed;
        z-index: 50;
        width: 100vw;
        height: 100vh;
        top: 0;
        background: rgb(0.5,0.5,0.5,0.5);
    }
    .modal-container {
        display: flex;
        flex-flow: column !important;
        flex-wrap: nowrap;
        overflow-y: hidden;
        max-height: 60vh;
        max-width: 85vw;
        margin: auto;
        background-color: black;
        border: 1px solid white;
        border-radius: 25px;
    }
    .modal-header {
        display: flex;
        justify-content: space-between;
    }
    .modal-close-button {
        margin: 16px;
        border-radius: 25px;
        width: 30px;
        height: 30px;
        text-align: center;
    }
    .modal-title {  
        margin-left: 24px;
        text-align: center;
    }
    .modal-body {
        display: flex;
        justify-content: center;
        overflow-y: auto;
    }
    .modal-container {
        display: flex;
        flex-flow: row;
        flex-wrap: nowrap;
        justify-content: space-between;
        box-shadow: -1px 1px 5px 0 cornflowerblue,1px -1px 5px 0 lightblue,1px 1px 5px 0 lightblue,-1px -1px 5px 0 cornflowerblue;
        min-width: 40vw;
    }
    
@media only screen and (-webkit-min-device-pixel-ratio: 2.75) {
    .modal-container {
        min-width: 95vw;
        max-height: 95vh;
    }
}

</style>
<div class="modal-overlay">
    <div class="modal-container">
        <div class="modal-header">
            <div class="modal-title">
                <h2>
                    <slot name="title"></slot>
                </h2>
            </div>
            <div>
                <button class="modal-close-button" onclick="this.getRootNode().host.hide()">X</button>
            </div>
        </div>
        <div class="modal-body">
            <slot></slot>
        </div>
        <div class="modal-footer">
            <div>
                <slot name="footerStart"></slot>
            </div>
            <div>
                <slot name="footerEnd"></slot>
            </div>
        </div>
    </div>
</div>
`

class ModalComponent extends HTMLElement {
    constructor() {
        super();
        const shadowRoot = this.attachShadow({mode: 'open'});
        shadowRoot.append(modalTemplate.content.cloneNode(true));
        
        //this.classList.add('hidden');
    }
    
    static get observedAttributes () {
        return ["visible"];
    }

    attributeChangedCallback(name, oldeValue, newValue) {
        if(name == "visible") {
            if(newValue) {
                this.show();
            } else {
                this.hide();
            }
        }
    } 
    connectedCallback() {
        // browser calls this method when the element is added to the document
        // (can be called many times if an element is repeatedly added/removed)
        console.log("connected");
        this.hide();
    }

    disconnectedCallback() {
        // browser calls this method when the element is removed from the document
        // (can be called many times if an element is repeatedly added/removed)
        console.log("disconnected");
    }

    adoptedCallback() {
        // called when the element is moved to a new document
        // (happens in document.adoptNode, very rarely used)
        console.log("adopted");
    }

    show() {
        this.style = 'display:revert;';
    }
    hide() {
        //this.classList.add('modal-hidden');
        this.style = 'display:none;';
        this.removeAttribute("visible");
    }
};

customElements.define('modal-component', ModalComponent);