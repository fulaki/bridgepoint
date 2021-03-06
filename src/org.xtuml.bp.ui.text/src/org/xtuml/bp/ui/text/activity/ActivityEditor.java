package org.xtuml.bp.ui.text.activity;
import java.io.StringReader;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IMarker;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.jface.text.Document;
import org.eclipse.jface.text.DocumentEvent;
import org.eclipse.jface.text.IDocument;
import org.eclipse.jface.text.IDocumentListener;
import org.eclipse.jface.text.IRegion;
import org.eclipse.jface.text.ITextDoubleClickStrategy;
import org.eclipse.jface.text.TextViewer;
import org.eclipse.jface.text.source.IAnnotationAccess;
import org.eclipse.jface.text.source.IAnnotationModel;
import org.eclipse.jface.viewers.StructuredSelection;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.FocusEvent;
import org.eclipse.swt.events.FocusListener;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.IEditorInput;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.FileEditorInput;
import org.eclipse.ui.texteditor.DefaultRangeIndicator;
import org.eclipse.ui.texteditor.IDocumentProvider;
import org.xtuml.bp.als.oal.OalLexer;
import org.xtuml.bp.core.Block_c;
import org.xtuml.bp.core.CorePlugin;
import org.xtuml.bp.core.Ooaofooa;
import org.xtuml.bp.core.Parsestatus_c;
import org.xtuml.bp.core.Pref_c;
import org.xtuml.bp.core.common.BridgePointPreferencesStore;
import org.xtuml.bp.core.common.NonRootModelElement;
import org.xtuml.bp.core.common.NullEditorInput;
import org.xtuml.bp.core.ui.Selection;
import org.xtuml.bp.core.util.EditorUtil;
import org.xtuml.bp.core.util.HierarchyUtil;
import org.xtuml.bp.ui.text.AbstractModelElementEditorInput;
import org.xtuml.bp.ui.text.DocumentProvider;
import org.xtuml.bp.ui.text.EditorConfiguration;
import org.xtuml.bp.ui.text.IModelElementEditorInputFactory;
import org.xtuml.bp.ui.text.IUITextHelpContextIds;
import org.xtuml.bp.ui.text.TextPlugin;
import org.xtuml.bp.ui.text.annotation.ActivityAnnotationAccess;
import org.xtuml.bp.ui.text.annotation.ActivityProblem;
import org.xtuml.bp.ui.text.editor.BPTextDefaultTextDoubleClickStategy;
import org.xtuml.bp.ui.text.editor.oal.OALEditor;

import antlr.RecognitionException;
import antlr.TokenStreamException;
import antlr.TokenStreamRecognitionException;

public class ActivityEditor extends OALEditor
{
  IDocumentListener listener = null;
  FocusListener focusListener = null;

  private String taggedContent;
  
  /**
   * Whether this editor has been closed, which an associated parse thread 
   * needs to know when finishing its wait to run, as it should abort at that
   * time if this is true. 
   */
  private boolean disposed = false;
  
  private static final String m_parseThreadName = "org.xtuml.bp.ui.text.parseThread."; //$NON-NLS-1$
  private volatile TolerantThread parseThread = null;
  private static int 
      HANGON_DURATION_original_value = 500, // in milli seconds
      HANGON_DURATION = HANGON_DURATION_original_value; 
  public static String getParseThreadName () { return m_parseThreadName; }

	private synchronized void accessParseThread(Runnable r, String name) {
	    if (parseThread != null && parseThread.isAlive()){
            parseThread.interrupt();
            try {
                parseThread.join();
            } catch (InterruptedException e) {}
	    } 
        
        // start a new parse thread
        parseThread = new TolerantThread(r, name, parseThread);
        parseThread.start();
	}

	
  protected static class ParseRunnable implements Runnable {
    public ActivityEditorInput m_ae_input;
    public IDocument m_document;
    public ActivityAnnotationModel m_myAnnotationModel;
    public NonRootModelElement m_modelElement;
    private boolean m_clearContext;
    /**
     * The editor (if any) which is editing the text being parsed. 
     */
    private ActivityEditor forEditor;
    
    /**
     * Constructor.
     * 
     * @param forEditor     See field.
     */
    public ParseRunnable(ActivityEditor forEditor)
    {
        this.forEditor = forEditor;
                                // When editing, the user could have changed
        m_clearContext = true;  // some visibility or declared a new element,
                                // so we want to recompute element visibility.
    }
    
    /**
     * Constructor.
     */
    public ParseRunnable() {
    	                        // When parsing outside an editor, we want
        m_clearContext = false; // to parse as quickly as possible, so we
                                // enable visibility caching.
    }
    
    public void run() {
        // if the model element whose activity text is to be parsed has been
        // deleted from its model, or this parse is for an editor which has
        // been disposed, then no parse should be performed
        if (m_modelElement == null  || m_modelElement.isOrphaned() || 
            (forEditor != null && forEditor.disposed)) return;
        
		Ooaofooa modelRoot = (Ooaofooa)m_modelElement.getModelRoot();
		OalLexer lexer = new OalLexer(
				new StringReader(m_document.get()));
		EditorTextParser parser = new EditorTextParser(modelRoot, lexer,
				m_myAnnotationModel, m_ae_input, m_document);
        boolean parseCompleted = false;
        boolean problemsFound = false;
		m_myAnnotationModel.beginReporting();
		try {
			// Parse the input expression
            parser.action(m_modelElement, m_clearContext);
            parseCompleted = true;
		} catch (TokenStreamException e) {
			Block_c.Clearcurrentscope(modelRoot,
					parser.m_oal_context.m_act_id);
			if (e instanceof TokenStreamRecognitionException) {
				TokenStreamRecognitionException tsre = (TokenStreamRecognitionException) e;
				parser.reportError(tsre.recog);
			} else {
				String errorMsg = "Internal parser error.  Token stream exception in parser.  OAL in this action home caused an exception in the parser."; //$NON-NLS-1$
				ActivityProblem ap =
					new ActivityProblem(
							errorMsg,
						IMarker.SEVERITY_ERROR,
						0,
						0,
						0,
						m_ae_input);

				m_myAnnotationModel.acceptProblem(ap);
				
				TextPlugin.logError(errorMsg, e);
			}
		} catch (RecognitionException e) {
			Block_c.Clearcurrentscope(modelRoot,
					parser.m_oal_context.m_act_id);
			parser.reportError(e);
		} catch (InterruptedException e) {
			// The parse was canceled, we don't need to report an 
			// error in this situation, but we do need to note that 
			// the parse did not complete (parseCompleted is false).
			Block_c.Clearcurrentscope(modelRoot,
					parser.m_oal_context.m_act_id);			
		} catch(Throwable t){
			String errorMsg = "Internal parser error.  Parsing thread interrupted pre-maturely.  OAL in this action home caused an exception in the parser.";   //$NON-NLS-1$
			Block_c.Clearcurrentscope(modelRoot,
					parser.m_oal_context.m_act_id);
			
			ActivityProblem ap =
				new ActivityProblem(
						errorMsg,
					IMarker.SEVERITY_ERROR,
					0,
					0,
					0,
					m_ae_input);

			m_myAnnotationModel.acceptProblem(ap);
		    //This throwable catches all the un-checked exceptions that occur in the thread, and logs them 
		    //appropriately.				    
		    TextPlugin.logError(errorMsg, t); //$NON-NLS-1$	
		}
			
        problemsFound = m_myAnnotationModel.containsProblems();
		m_myAnnotationModel.endReporting();
            
		// if there were no parse errors
        if (parseCompleted && !problemsFound) {
            // Relocatables are deprecated, so simply store the document.
            String taggedActionSemantics = m_document.get();
                
            // if the parse isn't being done for an editor (but rather, it is
            // being done as part of a parse-all)
            if (forEditor == null) {
	            m_myAnnotationModel.resetMarkers();
            }
                
            // if the parse is being done for an editor
            if (forEditor != null) {
                // store the tagged action semantics into the editor's
                // intermediate buffer, which is employed to keep unsaved 
                // user-entered changes in the editor from being prematurely
                // written to the model, where they might get persisted
                forEditor.taggedContent = taggedActionSemantics;
            }
        }

        // if the parse was done for an editor
        if (forEditor != null) {
            // if the parse was not completed, or there were parse errors,
            // then our intermediate buffer was not overwritten, so its 
            // contents are now out-of-date
            if (!parseCompleted || problemsFound) forEditor.taggedContent = null;
        }
	}
  };
  
  protected class Listener implements IDocumentListener
  {
    private ParseRunnable fRunnable = new ParseRunnable(ActivityEditor.this);

    public void documentChanged(DocumentEvent ev) {
			if (fOverviewRuler != null) fOverviewRuler.update();

            IDocumentProvider provider = getDocumentProvider();
			if (provider instanceof DocumentProvider) {
				Object input = getEditorInput();
				if (input instanceof ActivityEditorInput) {
			        NonRootModelElement modelElement = ((ActivityEditorInput) input).getModelElement();
			   		int toParse = AllActivityModifier.accessSuc_Pars( false, 0, modelElement );
			   		if ( toParse != Parsestatus_c.doNotParse )
			   		{
						DocumentProvider ae_provider = (DocumentProvider) provider;
						IAnnotationModel annotationModel = ae_provider.getAnnotationModel(input);
						if (annotationModel instanceof ActivityAnnotationModel) {
				      if (ev.getLength() != 0 || ev.getText().length() != 0 || ev.getOffset() != 0 || ev.getModificationStamp() != 0) {
							  fRunnable.m_ae_input = (ActivityEditorInput) input;
							  fRunnable.m_document = ae_provider.getDocument(input);
							  fRunnable.m_myAnnotationModel = (ActivityAnnotationModel) annotationModel;
						    fRunnable.m_modelElement = fRunnable.m_ae_input.getModelElement();
							  Ooaofooa.m_display = Display.getCurrent();
					      accessParseThread(fRunnable, getParseThreadName() + fRunnable.m_ae_input.getName());
	              Thread.yield();
						  }
			   		}
				}
			}
      }
		}
      public void documentAboutToBeChanged(DocumentEvent ev)
      {
      	// do nothing
      }
  };

  private class TolerantThread extends Thread{
	private int hangOnTime = HANGON_DURATION; 
	private volatile long lastTime;
	private volatile boolean isWaiting = true;
	private Thread previousThread;
    
	public TolerantThread(Runnable target, String name, Thread prevThread) {
		super(target, name);
		previousThread = prevThread;
		waitFromNow();		
	}

	
    public void waitFromNow(){
		accessLastTime(true, System.currentTimeMillis());		
	}
	
	public void run(){
    	// If user has selected not to parse on edit, then ignore
    	if (!Pref_c.Getboolean(BridgePointPreferencesStore.ENABLE_PARSE_ON_ACTIVITY_EDITS)) {
    		return;
    	}
		
        if(previousThread != null && previousThread.isAlive())
		{
			try {
				previousThread.interrupt();
				previousThread.join();		
			} 
			catch (InterruptedException e) {
				return;
			} 
		}		
		while((System.currentTimeMillis() - accessLastTime(false, 0)) < hangOnTime){
			try {				
				Thread.sleep(hangOnTime);
			} catch (InterruptedException e) {
				return;
			}
		}
		accessIsWaiting(true, false);
        super.run();		
	}
	
	private synchronized boolean accessIsWaiting(boolean write, boolean newValue) {
		if ( write )
		{
			isWaiting = newValue;
		}
		return isWaiting;
	}
	/**
	 * @param aLastTime The lastTime to set.
	 */
	private synchronized long accessLastTime(boolean write, long newValue) {
		if ( write )
		{
			lastTime = newValue;
		}
		return lastTime;
	}
}
  public ActivityEditor()
  {
    super();
    setDocumentProvider(new DocumentProvider());
    setRangeIndicator(new DefaultRangeIndicator());
    
  }
  
  // This is actually a protected method
  protected void doSetInput(IEditorInput input) throws CoreException
  {
      // if this editor is already editing a document
      IDocumentProvider provider = getDocumentProvider();
      IDocument document = provider.getDocument(getEditorInput());
      if (document != null) {
          // if we added our listener to the document
          if (listener != null) {
              // remove it, as the document is going away
              document.removeDocumentListener(listener);
          }
      }

      IEditorInput modelInput = null;

	if(input instanceof ActivityEditorInput){
  		modelInput = input;
	}else if(input instanceof AbstractModelElementEditorInput){
		// since AbstractModelElementEditorInput is instance of FileEditorInput, we need to put this check
		// TODO issue 720 will remove this check.
		throw new PartInitException("EditorInput of type " + input.getClass() + " not supported");
	}else if(input instanceof FileEditorInput){
  		IFile file = ((FileEditorInput)input).getFile();
  		IModelElementEditorInputFactory factory = (IModelElementEditorInputFactory)PlatformUI.getWorkbench().getElementFactory(ActivityEditorInput.FACTORY_ID);
  		modelInput = factory.createInstance(file);
  	}else if(input instanceof NullEditorInput){
  		modelInput = input;
  	}else{
  	  	throw new PartInitException("EditorInput of type " + input.getClass() + " not supported");
  	}
  	
	super.doSetInput(modelInput);

    // add a new instance of our listener to the document for 
    // the new input
    document = provider.getDocument(modelInput);
    if(document != null){
        listener = new Listener();
        document.addDocumentListener(listener);
    }
  }

  /*
   * @see IWorkbenchPart#createPartControl(Composite)
   */
  public void createPartControl(Composite parent)
  {
    setSourceViewerConfiguration(new EditorConfiguration(this, TextPlugin.getDefault().getSyntaxHighlightingPreferences(), this));
    super.createPartControl(parent);
    StyledText textWidget = null;
    if (getSourceViewer() != null && getSourceViewer().getTextWidget() != null)
        textWidget = getSourceViewer().getTextWidget();
    if ( textWidget != null )
    {
        if ( focusListener != null )
        {
            textWidget.removeFocusListener(focusListener);
            focusListener = null;
        }
        focusListener = new FocusListener() {
            public void focusGained(FocusEvent ev) {
                Object selObj = ((ActivityEditorInput)getEditorInput()).getModelElement();
                if(selObj != null){
	                StructuredSelection sel = new StructuredSelection(selObj);
					Selection.getInstance().setSelection(sel);
                }
                EditorUtil.refreshEditorTab();
            }
            public void focusLost(FocusEvent ev) { /* do nothing */ }
        };
        textWidget.addFocusListener(focusListener);
        // add F1 context help for the editor
        PlatformUI.getWorkbench().getHelpSystem().setHelp(textWidget, IUITextHelpContextIds.activityEditorId);
    }
  }
  public void dispose()
  {
    if ( listener != null )
    {
        IDocumentProvider provider = getDocumentProvider();
        IDocument document = provider.getDocument(getEditorInput());
        document.removeDocumentListener(listener);
        listener = null;
    }
    if ( focusListener != null )
    {
        if (getSourceViewer() != null && getSourceViewer().getTextWidget() != null)
            getSourceViewer().getTextWidget().removeFocusListener(focusListener);
        focusListener = null;
    }
 
    // disconnect our model change listener
    ActivityEditorInput input = (ActivityEditorInput)getEditorInput();

    disposed = true;
    
    // if this editor has a parse thread spinning
    if (parseThread != null && parseThread.isAlive()) {
        // if the parse thread is waiting to run, end the wait, so it can 
        // notice this editor is disposed and die
        parseThread.interrupt();
    }
    
    super.dispose();
  }
  public boolean isSaveAsAllowed()
  {
    return false;
  }

  protected IAnnotationAccess createAnnotationAccess()
  {
    return new ActivityAnnotationAccess();
  }
  
  @Override
  public Image getTitleImage() {
	  Object element = ((ActivityEditorInput)this.getEditorInput()).getModelElement();
	  return CorePlugin.getImageFor(element);
  }
  
  @Override
  public String getTitleToolTip() {
	  Object element = ((ActivityEditorInput)this.getEditorInput()).getModelElement();
	  if (element == null)
		  return "";
	  return HierarchyUtil.Getpath(element);
  }
  
/**
 * only for use by unit test code
 */
public TextViewer getTextViewer() {
    return (TextViewer) getSourceViewer();
}
public void waitForParseThread()
{
    if (parseThread != null && parseThread.isAlive())
    {
        try{
        	parseThread.join();
        }
        catch( InterruptedException ie)
        {
            TextPlugin.logError("UI thread interrupted while waiting for parse thread to exit", ie); //$NON-NLS-1$
        }
    }
}

    /* (non-Javadoc)
     * @see org.eclipse.ui.ISaveablePart#doSave(org.eclipse.core.runtime.IProgressMonitor)
     */
    public void doSave(IProgressMonitor progressMonitor)
    {
        waitForParseThread();
        super.doSave(progressMonitor);
        // write the contents of this editor's intermediate buffer 
        // (containing the action semantics)
        // or the document text (if the intermediate buffer is empty due 
        // to one or more parse errors) into the model
        ActivityEditorInput input = (ActivityEditorInput)getEditorInput();
        input.doSaveDocument(progressMonitor, input.getModelElement(),
            (taggedContent != null) ? new Document(taggedContent) : 
                getDocumentProvider().getDocument(input), 
            true);
    }
    
    /**
     * Removes the dirty marker from this editor, if there is one.
     */
    private void setAsNotDirty() 
    {
        if (!isDirty()) return;
        
        ((DocumentProvider)getDocumentProvider()).setAsNotDirty(getEditorInput());
    }
    
    /**
     * See HANGON_DURATION field.  Tests might want to lower the value of this 
     * field to be able to complete faster.
     */
    public static void setWaitBeforeParse(int duration)
    {
        HANGON_DURATION = duration;
    }
    
    /**
     * Resets the HANGON_DURATION field to its original value. 
     */
    public static void resetWaitBeforeParse()
    {
        HANGON_DURATION = HANGON_DURATION_original_value;
    }

	public IRegion findWord(IDocument doc, int offset) {
		ITextDoubleClickStrategy doubleClickStrategy = getSourceViewerConfiguration()
				.getDoubleClickStrategy(getSourceViewer(), taggedContent);
		BPTextDefaultTextDoubleClickStategy dblClckStrategy = (BPTextDefaultTextDoubleClickStategy) doubleClickStrategy;
		return dblClckStrategy.findWord(doc, offset);
	}
}
