/// <reference path="oskKey.ts" />

namespace com.keyman.osk {
  let Codes = com.keyman.text.Codes;

  export class OSKBaseKey extends OSKKey {
    private capLabel: HTMLDivElement;
    public readonly row: OSKRow;

    constructor(spec: OSKKeySpec, layer: string, row: OSKRow) {
      super(spec, layer);
      this.row = row;
    }

    getId(): string {
      // Define each key element id by layer id and key id (duplicate possible for SHIFT - does it matter?)
      return this.spec.elementID;
    }

    getCoreId(): string {
      return this.spec.coreID;
    }

    getBaseId(): string {
      return this.spec.baseKeyID;
    }

    // Produces a small reference label for the corresponding physical key on a US keyboard.
    private generateKeyCapLabel(): HTMLDivElement {
      // Create the default key cap labels (letter keys, etc.)
      var x = Codes.keyCodes[this.spec.baseKeyID];
      switch(x) {
        // Converts the keyman key id code for common symbol keys into its representative ASCII code.
        // K_COLON -> K_BKQUOTE
        case 186: x=59; break;
        case 187: x=61; break;
        case 188: x=44; break;
        case 189: x=45; break;
        case 190: x=46; break;
        case 191: x=47; break;
        case 192: x=96; break;
        // K_LBRKT -> K_QUOTE
        case 219: x=91; break;
        case 220: x=92; break;
        case 221: x=93; break;
        case 222: x=39; break;
        default:
          // No other symbol character represents a base key on the standard QWERTY English layout.
          if(x < 48 || x > 90) {
            x=0;
          }
      }

      let q = document.createElement('div');
      q.className='kmw-key-label';
      if(x > 0) {
        q.innerHTML=String.fromCharCode(x);
      } else {
        // Keyman-only virtual keys have no corresponding physical key.
        // So, no text for the key-cap.
      }
      return q;
    }

    private processSubkeys(btn: KeyElement, vkbd: VisualKeyboard) {
      // Add reference to subkey array if defined
      var bsn: number, bsk=btn['subKeys'] = this.spec['sk'];
      // Transform any special keys into their PUA representations.
      for(bsn=0; bsn<bsk.length; bsn++) {
        if(bsk[bsn]['sp'] == '1' || bsk[bsn]['sp'] == '2') {
          var oldText=bsk[bsn]['text'];
          bsk[bsn]['text']=this.renameSpecialKey(oldText, vkbd);
        }

        // If a subkey doesn't have a defined layer property, copy it from the base key's layer by default.
        if(!bsk[bsn].layer) {
          bsk[bsn].layer = btn.key.layer
        }
      }

      // If a subkey array is defined, add an icon
      var skIcon = document.createElement('div');
      skIcon.className='kmw-key-popup-icon';
      //kDiv.appendChild(skIcon);
      btn.appendChild(skIcon);
    }

    construct(vkbd: VisualKeyboard): HTMLDivElement {
      let spec = this.spec;

      let kDiv = document.createElement('div');
      kDiv.className='kmw-key-square';

      let btnEle = document.createElement('div');
      let btn = this.btn = link(btnEle, new KeyData(this, spec['id']));

      // Set button class
      this.setButtonClass();

      // Add the (US English) keycap label for layouts requesting display of underlying keys
      let keyCap = this.capLabel = this.generateKeyCapLabel();
      btn.appendChild(keyCap);

      // Define each key element id by layer id and key id (duplicate possible for SHIFT - does it matter?)
      btn.id=this.getId();

      // Make sure the key text is the element's first child - processSubkeys()
      // will add an extra element if subkeys exist, which can interfere with
      // keyboard/language name display on the space bar!
      btn.appendChild(this.label = this.generateKeyText(vkbd));

      // Handle subkey-related tasks.
      if(typeof(spec['sk']) != 'undefined' && spec['sk'] != null) {
        this.processSubkeys(btn, vkbd);
      } else {
        btn['subKeys']=null;
      }

      // Add text to button and button to placeholder div
      kDiv.appendChild(btn);

      // The 'return value' of this process.
      return this.square = kDiv;
    }

    public refreshLayout(vkbd: VisualKeyboard) {
      let key = this.spec as keyboards.ActiveKey;
      this.square.style.width = vkbd.layoutWidth.scaledBy(key.proportionalWidth).styleString;
      this.square.style.marginLeft = vkbd.layoutWidth.scaledBy(key.proportionalPad).styleString;
      this.btn.style.width = vkbd.usesFixedWidthScaling ? this.square.style.width : '100%';
      
      if(vkbd.usesFixedHeightScaling) {
        // Matches its row's height.
        this.square.style.height = vkbd.layoutHeight.scaledBy(this.row.heightFraction).styleString;
      } else {
        this.square.style.height = '100%'; // use the full row height
      }

      super.refreshLayout(vkbd);

      let util = com.keyman.singleton.util;
      const device = vkbd.device;
      const resizeLabels = (device.OS == utils.OperatingSystem.iOS &&
                            device.formFactor == utils.FormFactor.Phone
                            && util.landscapeView());

      // Rescale keycap labels on iPhone (iOS 7)
      if(resizeLabels && this.capLabel) {
        this.capLabel.style.fontSize = '6px';
      }
    }

    public get displaysKeyCap(): boolean {
      return this.capLabel && this.capLabel.style.display == 'block';
    }

    public set displaysKeyCap(flag: boolean) {
      if(!this.capLabel) {
        throw new Error("Key element not yet constructed; cannot display key cap");
      }
      this.capLabel.style.display = flag ? 'block' : 'none';
    }
  }
}
