function toggleMotionRandomSettings(e,t){const o=document.getElementById("motionOffsetRandom"+e+t).checked,n=document.getElementById("motionAmplitudeRandom"+e+t).checked,i=document.getElementById("motionPeriodRandom"+e+t).checked,a=document.getElementById("motionPhaseRandom"+e+t).checked;Utils.toggleControlVisibilityByID("motionOffsetRandomMinRow"+e+t,o),Utils.toggleControlVisibilityByID("motionOffsetRandomMaxRow"+e+t,o),Utils.toggleControlVisibilityByID("motionAmplitudeRandomMinRow"+e+t,n),Utils.toggleControlVisibilityByID("motionAmplitudeRandomMaxRow"+e+t,n),Utils.toggleControlVisibilityByID("motionPeriodRandomMinRow"+e+t,i),Utils.toggleControlVisibilityByID("motionPeriodRandomMaxRow"+e+t,i),Utils.toggleControlVisibilityByID("motionPhaseRandomMinRow"+e+t,a),Utils.toggleControlVisibilityByID("motionPhaseRandomMaxRow"+e+t,a),toggleMotionRandomMinMaxSettingsSettings(e,t)}function toggleMotionRandomSettingsElement(e,t,o){Utils.toggleElementShown(e,o),Utils.toggleElementShown(t,o)}function toggleMotionRandomMinMaxSettingsSettings(e,t){const o=document.getElementById("motionOffsetRandom"+e+t).checked,n=document.getElementById("motionAmplitudeRandom"+e+t).checked,i=document.getElementById("motionPeriodRandom"+e+t).checked,a=document.getElementById("motionPhaseRandom"+e+t).checked;Utils.toggleControlVisibilityByID("motionRandomChangeMinRow"+e+t,o||n||i||a),Utils.toggleControlVisibilityByID("motionRandomChangeMaxRow"+e+t,o||n||i||a)}function setMotionEnabledClicked(){systemInfo.motionEnabled=!systemInfo.motionEnabled,sendTCode(systemInfo.motionEnabled?"#motion-enable":"#motion-disable"),MotionGenerator.setEnabledStatus()}function setMotionPeriodRandomClicked(e,t,o){var n=document.getElementById("motionPeriodRandom"+e+t).checked;const i=getMotionChannelIndex(e,o);motionProviderSettings.motionProfiles[e].channels[i].periodRan=n,Utils.toggleControlVisibilityByID("motionPeriodRandomMinRow"+e+t,motionProviderSettings.motionProfiles[e].channels[i].periodRan),Utils.toggleControlVisibilityByID("motionPeriodRandomMaxRow"+e+t,motionProviderSettings.motionProfiles[e].channels[i].periodRan),toggleMotionRandomMinMaxSettingsSettings(e,t),MotionGenerator.updateSettings(e,i)}function setMotionAmplitudeRandomClicked(e,t,o){var n=document.getElementById("motionAmplitudeRandom"+e+t).checked;const i=getMotionChannelIndex(e,o);motionProviderSettings.motionProfiles[e].channels[i].ampRan=n,Utils.toggleControlVisibilityByID("motionAmplitudeRandomMinRow"+e+t,n),Utils.toggleControlVisibilityByID("motionAmplitudeRandomMaxRow"+e+t,n),toggleMotionRandomMinMaxSettingsSettings(e,t),MotionGenerator.updateSettings(e,i)}function setMotionOffsetRandomClicked(e,t,o){var n=document.getElementById("motionOffsetRandom"+e+t).checked;const i=getMotionChannelIndex(e,o);motionProviderSettings.motionProfiles[e].channels[i].offsetRan=n,Utils.toggleControlVisibilityByID("motionOffsetRandomMinRow"+e+t,n),Utils.toggleControlVisibilityByID("motionOffsetRandomMaxRow"+e+t,n),toggleMotionRandomMinMaxSettingsSettings(e,t),MotionGenerator.updateSettings(e,i)}function setMotionPhaseRandomClicked(e,t,o){var n=document.getElementById("motionPhaseRandom"+e+t).checked;const i=getMotionChannelIndex(e,o);motionProviderSettings.motionProfiles[e].channels[i].phaseRan=n,Utils.toggleControlVisibilityByID("motionPhaseRandomMinRow"+e+t,n),Utils.toggleControlVisibilityByID("motionPhaseRandomMaxRow"+e+t,n),toggleMotionRandomMinMaxSettingsSettings(e,t),MotionGenerator.updateSettings(e,i)}function setMotionProfileDefault(){motionProviderSettings.motionDefaultProfileIndex=getMotionProfileSelectedIndex(),MotionGenerator.updateSettings(1)}MotionGenerator={channelTemplates:[],setup(){const e=document.getElementById("motionProfiles");removeAllChildren(e),channelTemplates=[],e.value=motionProviderSettings.motionDefaultProfileIndex;var t=getChannelMap();motionProviderSettings.motionProfiles.forEach(((e,o)=>{addMotionProfileOption(o,e);var n=document.createElement("div");n.classList.add("formTable"),n.style="box-shadow: none; width: 100%;",n.id="channelContainer"+o,n.name="channelContainer";var i=document.createElement("div"),a=document.createElement("div");a.classList.add("tHeader"),i.appendChild(a);let d=Utils.createTextFormRow(0,"Profile name","motionProfileName"+o,e.name,31,function(e){setMotionGeneratorName(e)}.bind(this,o));i.appendChild(d.row);for(var s=0;s<t.length;s++){const e=s;var l=t[s];if(!userSettings.sr6Mode&&l.sr6Only)continue;var m=l.channel,r=l.channelName,c=motionProviderSettings.motionProfiles[o].channels.findIndex((e=>e.name==m));const n=motionProviderSettings.motionProfiles[o].channels[c];var f=document.createElement("div");f.classList.add("tRow"),f.classList.add("motion-profile-edit-channel-header");var g=document.createElement("div");g.classList.add("tCell");var h=document.createElement("div");h.classList.add("tCell");var u=document.createElement("span");u.for="motionChannelCheckbox"+o+e,u.innerText=r+" ("+m+")";var M=document.createElement("input");M.id="motionChannelCheckbox"+o+e,M.type="checkbox",M.classList.add("motion-profile-edit-channel-header-checkbox"),M.value=`{"channelName": "${m}", "profileIndex": ${o}, "channelIndex": ${e}}`,M.checked=c>-1,M.onclick=function(){var e=JSON.parse(this.value);const t=getMotionChannelIndex(e.profileIndex,e.channelName);if(t>-1){if(!confirm("In order to save memory, this action will set the channel settings for this profile to DEFAULT. Are you sure?"))return void(this.checked=!0);motionProviderSettings.motionProfiles[e.profileIndex].channels.splice(t,1)}this.checked?motionProviderSettings.motionProfiles[e.profileIndex].channels.push(getDefaultMotionChannel(e.channelName)):Utils.toggleControlVisibilityByID("channelTable"+e.profileIndex+e.channelIndex,!1,!0);const n=document.getElementById("motionChannelEditButton"+e.profileIndex+e.channelIndex);Utils.toggleElementShown(n,this.checked),n.disabled=!this.checked,MotionGenerator.updateSettings(o,e.channelIndex)};var p=document.createElement("button");p.id="motionChannelEditButton"+o+e,p.innerText="Edit",Utils.toggleElementShown(p,M.checked),p.disabled=!M.checked,p.onclick=function(){Utils.toggleControlVisibilityByName("channelTable",!1,!0);const t=this.classList.contains("button-pressed"),n=document.getElementsByClassName("button-pressed");for(var i=0;i<n.length;i++)n[i].classList.remove("button-pressed");t||(Utils.toggleControlVisibilityByID("channelTable"+o+e,!0,!0),this.classList.add("button-pressed"))},g.appendChild(u),g.appendChild(M),h.appendChild(p),f.appendChild(g),f.appendChild(h),i.appendChild(f);const a=document.createElement("div");a.id="channelTable"+o+e,a.setAttribute("name","channelTable"),a.classList.add("formTable"),Utils.toggleElementShown(a,!1,!0),a.style="box-shadow: none; width: auto; margin: 0; padding: 0;";const d=document.createElement("div");let R=Utils.createNumericFormRow(0,"Update rate (ms)","motionUpdate"+o+e,n?n.update:100,0,2147483647,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m));R.title="This is the time in between updates that gives the system time to process other tasks. (DO NOT SET TOO LOW ON ESP32!)\n                It may be best to just leave at default.",d.appendChild(R.row),R=Utils.createNumericFormRow(0,"Period (ms)","motionPeriod"+o+e,n?n.period:2e3,0,2147483647,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),R.title="This is the time it takes to make one stroke.\n                Lower is faster movement.",d.appendChild(R.row);const P=!!n&&n.periodRan;R=Utils.createCheckboxFormRow(0,"Period random","motionPeriodRandom"+o+e,P,function(e,t,o){setMotionPeriodRandomClicked(e,t,o)}.bind(this,o,e,m)),R.title="This is the time it takes to make one stroke.\n                Lower is faster movement.",d.appendChild(R.row),R=Utils.createNumericFormRow("motionPeriodRandomMinRow"+o+e,"Min (ms)","motionPeriodRandomMin"+o+e,n?n.periodMin:500,0,2147483647,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row);let v=R.input,x=R;R=Utils.createNumericFormRow("motionPeriodRandomMaxRow"+o+e,"Max (ms)","motionPeriodRandomMax"+o+e,n?n.periodMax:2e3,0,2147483647,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),setElementsIntMinAndMax(v,R.input),toggleMotionRandomSettingsElement(x.row,R.row,P),R=Utils.createNumericFormRow(0,"Amplitude (%)","motionAmplitude"+o+e,n?n.amp:60,0,100,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row);const S=!!n&&n.ampRan;R=Utils.createCheckboxFormRow(0,"Amplitude random","motionAmplitudeRandom"+o+e,S,function(e,t,o){setMotionAmplitudeRandomClicked(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),R=Utils.createNumericFormRow("motionAmplitudeRandomMinRow"+o+e,"Min (%)","motionAmplitudeRandomMin"+o+e,n?n.ampMin:20,0,100,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),v=R.input,x=R,R=Utils.createNumericFormRow("motionAmplitudeRandomMaxRow"+o+e,"Max (%)","motionAmplitudeRandomMax"+o+e,n?n.ampMax:60,0,100,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),setElementsIntMinAndMax(v,R.input),toggleMotionRandomSettingsElement(x.row,R.row,S),R=Utils.createNumericFormRow(0,"Offset (tcode)","motionOffset"+o+e,n?n.offset:5e3,0,9999,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row);const C=!!n&&n.offsetRan;R=Utils.createCheckboxFormRow(0,"Offset random","motionOffsetRandom"+o+e,C,function(e,t,o){setMotionOffsetRandomClicked(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),R=Utils.createNumericFormRow("motionOffsetRandomMinRow"+o+e,"Min (tcode)","motionOffsetRandomMin"+o+e,n?n.offsetMin:3e3,0,9999,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),v=R.input,x=R,R=Utils.createNumericFormRow("motionOffsetRandomMaxRow"+o+e,"Max (tcode)","motionOffsetRandomMax"+o+e,n?n.offsetMax:7e3,0,9999,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),setElementsIntMinAndMax(v,R.input),toggleMotionRandomSettingsElement(x.row,R.row,C),R=Utils.createNumericFormRow(0,"Phase (degree)","motionPhase"+o+e,n?n.phase:0,0,180,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row);const I=!!n&&n.phaseRan;R=Utils.createCheckboxFormRow(0,"Phase random","motionPhaseRandom"+o+e,I,function(e,t,o){setMotionPhaseRandomClicked(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),R=Utils.createNumericFormRow("motionPhaseRandomMinRow"+o+e,"Min (degree)","motionPhaseRandomMin"+o+e,n?n.phaseMin:0,0,180,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),v=R.input,x=R,R=Utils.createNumericFormRow("motionPhaseRandomMaxRow"+o+e,"Max (degree)","motionPhaseRandomMax"+o+e,n?n.phaseMax:180,0,180,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),setElementsIntMinAndMax(v,R.input),toggleMotionRandomSettingsElement(x.row,R.row,I),R=Utils.createNumericFormRow("motionRandomChangeMinRow"+o+e,"Random change max (ms)","motionRandomChangeMin"+o+e,n?n.ranMin:3e3,0,2147483647,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),v=R.input,x=R,R=Utils.createNumericFormRow("motionRandomChangeMaxRow"+o+e,"Random change max (ms)","motionRandomChangeMax"+o+e,n?n.ranMax:3e4,0,2147483647,function(e,t,o){setMotionGeneratorSettings(e,t,o)}.bind(this,o,e,m)),d.appendChild(R.row),setElementsIntMinAndMax(v,R.input),toggleMotionRandomSettingsElement(x.row,R.row,I||S||C||P);document.createElement("button").onclick=function(e,t,o){setMotionGeneratorSettingsDefault(e,t,o)}.bind(this,o,e,m),i.appendChild(a),a.appendChild(d)}n.appendChild(i),channelTemplates.push(n)}));const o=document.createElement("button");o.innerText="Set selected profile as default",o.onclick=setMotionProfileDefault,e.appendChild(o);const n=document.createElement("button");n.innerText="Edit device range limits",n.onclick=DeviceRangeSlider.show,e.appendChild(n),selectMotionProfile(systemInfo.motionSelectedProfileIndex),this.setEnabledStatus()},setEnabledStatus(){var e=document.getElementById("motionEnabledToggle");e.innerText=systemInfo.motionEnabled?"Stop":"Start",systemInfo.motionEnabled?(e.classList.add("button-toggle-stop"),e.classList.remove("button-toggle-start")):(e.classList.remove("button-toggle-stop"),e.classList.add("button-toggle-start"))},updateSettings(e,t,o){e&&t&&this.setEdited(e,t),updateUserSettings(o,EndPointType.MotionProfile.uri,motionProviderSettings)},setEdited(e,t){e>-1&&(motionProviderSettings.motionProfiles[e].edited=!0),t>-1&&(motionProviderSettings.motionProfiles[e].channels[t].edited=!0)}};var validateGeneratorSettingsDebounce,setMotionGeneratorNameDebounce,selectedMotionProfileIndex=0;function getMotionProfileSelectedIndex(){return selectedMotionProfileIndex}function setSelectedMotionProfileIndex(e){selectedMotionProfileIndex=parseInt(e)}function clearMotionProfileSelection(){var e=document.getElementsByName("motionProfileButton");e&&e.forEach((e=>e.classList.remove("button-pressed")))}function selectMotionProfile(e,t){clearMotionProfileSelection(),setSelectedMotionProfileIndex(e);document.getElementById(`motionProfile${e}`).classList.add("button-pressed"),t&&sendTCode(`#motion-profile-set:${e+1}`)}function editMotionProfile(e){const t=document.getElementById("motionChannelsModal");removeAllChildren(t),t.appendChild(channelTemplates[e]),setMotionGeneratorSettingsProfile(e);const o=document.createElement("span");o.innerText="Edit profile",o.setAttribute("slot","title"),t.appendChild(o),t.show()}function setMotionGeneratorSettings(e,t,o){validateGeneratorSettingsDebounce&&clearTimeout(validateGeneratorSettingsDebounce),validateGeneratorSettingsDebounce=setTimeout(function(e,t,o){var n=motionProviderSettings.motionProfiles[e].channels.findIndex((e=>e.name==o));if(-1!=i){var i=motionProviderSettings.motionProfiles[e].channels[n];validateIntControl("motionUpdate"+e+t,i,"update"),validateIntControl("motionPeriod"+e+t,i,"period"),validateIntControl("motionAmplitude"+e+t,i,"amp"),validateIntControl("motionOffset"+e+t,i,"offset"),validateIntControl("motionPhase"+e+t,i,"phase"),setIntMinAndMax("motionPeriodRandomMin"+e+t,"motionPeriodRandomMax"+e+t),validateIntControl("motionPeriodRandomMin"+e+t,i,"periodMin"),validateIntControl("motionPeriodRandomMax"+e+t,i,"periodMax"),setIntMinAndMax("motionAmplitudeRandomMin"+e+t,"motionAmplitudeRandomMax"+e+t),validateIntControl("motionAmplitudeRandomMin"+e+t,i,"ampMin"),validateIntControl("motionAmplitudeRandomMax"+e+t,i,"ampMax"),setIntMinAndMax("motionOffsetRandomMin"+e+t,"motionOffsetRandomMax"+e+t),validateIntControl("motionOffsetRandomMin"+e+t,i,"offsetMin"),validateIntControl("motionOffsetRandomMax"+e+t,i,"offsetMax"),setIntMinAndMax("motionPhaseRandomMin"+e+t,"motionPhaseRandomMax"+e+t),validateFloatControl("motionPhaseRandomMin"+e+t,i,"phaseMin"),validateFloatControl("motionPhaseRandomMax"+e+t,i,"phaseMax"),setIntMinAndMax("motionRandomChangeMin"+e+t,"motionRandomChangeMax"+e+t),validateIntControl("motionRandomChangeMin"+e+t,i,"ranMin"),validateIntControl("motionRandomChangeMax"+e+t,i,"ranMax"),MotionGenerator.updateSettings(1)}else showError("There was an error finding a channel with name: "+o)}.bind(this,e,t,o),3e3)}function setMotionGeneratorName(e){setMotionGeneratorNameDebounce&&clearTimeout(setMotionGeneratorNameDebounce),setMotionGeneratorNameDebounce=setTimeout(function(e){validateStringControl("motionProfileName"+e,motionProviderSettings.motionProfiles[e],"name")&&(updateMotionProfileName(e),MotionGenerator.updateSettings(1))}.bind(this,e),3e3)}function addMotionProfileOption(e,t){document.getElementsByName("channelContainer").forEach((e=>{Utils.toggleElementShown(e,!1)}));const o=document.getElementById("motionProfiles");var n=document.createElement("button");n.id=`motionProfile${e}`,n.name="motionProfileButton",n.value=e,n.innerText=t.name,n.style.width="80%",n.onclick=function(e){selectMotionProfile(e,!0)}.bind(this,e);var i=document.createElement("button");i.id=`motionProfileEdit${e}`,i.name="motionProfileEditButton",i.value=e,i.innerText="Edit",i.style.width="20%",i.onclick=function(e){editMotionProfile(e)}.bind(this,e),o.appendChild(n),o.appendChild(i)}function updateMotionProfileName(e){const t=document.getElementsByName("motionProfileButton");for(var o=getMotionProfileSelectedIndex(),n=0;n<t.length;n++)if(t[n].value==e){o=n;break}t[o].innerText=motionProviderSettings.motionProfiles[e].name}function setMotionGeneratorSettingsDefault(e,t,o){confirm(`Are you sure you want to set the current channel '${o}' in profile '${motionProviderSettings.motionProfiles[e].name}' to the default settings?`)&&(setProfileMotionGeneratorSettingsDefault(e,t,o),MotionGenerator.updateSettings(1))}function getMotionChannelIndex(e,t){return motionProviderSettings.motionProfiles[e].channels.findIndex((e=>e.name==t))}function getDefaultMotionChannel(e){return{name:e,update:100,period:2e3,amp:60,offset:5e3,periodRan:!1,periodMin:500,periodMax:2e3,ampRan:!1,ampMin:20,ampMax:60,offsetRan:!1,offsetMin:3e3,offsetMax:7e3,ranMin:3e3,ranMax:3e4,phase:0,phaseRan:!1,phaseMin:0,phaseMax:180,reverse:!1}}function setProfileMotionGeneratorSettingsDefault(e,t,o){motionProviderSettings.motionProfiles[e].name="Profile "+(e+1),motionProviderSettings.motionProfiles[e].channels[t]=getDefaultMotionChannel(o),setMotionGeneratorSettingsProfile(e)}function setMotionGeneratorSettingsProfile(e){document.getElementById("motionProfileName"+e).value=motionProviderSettings.motionProfiles[e].name;const t=getChannelMap();for(var o=0;o<t.length;o++){const n=o,i=t[o];if(!userSettings.sr6Mode&&i.sr6Only)continue;const a=getMotionChannelIndex(e,i.channel),d=a>-1,s=getDefaultMotionChannel();document.getElementById("motionUpdate"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].update:s.update,document.getElementById("motionPeriod"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].period:s.period,document.getElementById("motionAmplitude"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].amp:s.amp,document.getElementById("motionOffset"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].offset:s.offset,document.getElementById("motionPhase"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].phase:s.phase,document.getElementById("motionPhaseRandom"+e+n).checked=d?motionProviderSettings.motionProfiles[e].channels[a].phaseRan:s.phaseRan,document.getElementById("motionPhaseRandomMin"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].phaseMin:s.phaseMin,document.getElementById("motionPhaseRandomMax"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].phaseMax:s.phaseMax,document.getElementById("motionPeriodRandom"+e+n).checked=d?motionProviderSettings.motionProfiles[e].channels[a].periodRan:s.periodRan,document.getElementById("motionPeriodRandomMin"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].periodMin:s.periodMin,document.getElementById("motionPeriodRandomMax"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].periodMax:s.periodMax,document.getElementById("motionAmplitudeRandom"+e+n).checked=d?motionProviderSettings.motionProfiles[e].channels[a].ampRan:s.ampRan,document.getElementById("motionAmplitudeRandomMin"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].ampMin:s.ampMin,document.getElementById("motionAmplitudeRandomMax"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].ampMax:s.ampMax,document.getElementById("motionOffsetRandom"+e+n).checked=d?motionProviderSettings.motionProfiles[e].channels[a].offsetRan:s.offsetRan,document.getElementById("motionOffsetRandomMin"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].offsetMin:s.offsetMin,document.getElementById("motionOffsetRandomMax"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].offsetMax:s.offsetMax,document.getElementById("motionRandomChangeMin"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].ranMin:s.ranMin,document.getElementById("motionRandomChangeMax"+e+n).value=d?motionProviderSettings.motionProfiles[e].channels[a].ranMax:s.ranMax,toggleMotionRandomSettings(e,n)}}